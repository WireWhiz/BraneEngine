#include "assetHttpServer.h"
#include "../assetProcessing/AssetBuilder.h"
#include "assetServer/gltf/gltfLoader.h"
#include "utility/hex.h"

AssetHttpServer::AssetHttpServer(const std::string& domain, bool useHttps, Database& db, AssetManager& am, FileManager& fm): HTTPServer(domain, useHttps), _db(db), _am(am), _fm(fm)
{
	setUpAPICalls();
	setUpListeners();
}

void AssetHttpServer::setUpListeners()
{
	_server->Get("/app?(/.*)", [this](const httplib::Request &req, httplib::Response &res){
		if(_sessions.count(getCookie("session_id", req)))
		{
			serveFile(req, res, _files["/app"]);
		}
		else
		{
			res.set_content("Must be logged in to see this page.", "text/html");
			res.status = 403;
		}
	});
	_server->Get("/(.*)", [this](const httplib::Request &req, httplib::Response &res){
		std::cout << "Request: " << req.path <<std::endl;
		if(_files.count(req.path))
		{
			serverFile& file = _files[req.path];
			std::string sessionID = getCookie("session_id", req);
			if(_sessions.count(sessionID) && _sessions[sessionID].userAuthorized(file))
				serveFile(req, res, _files[req.path]);
			else if(file.authLevel == "public")
				serveFile(req, res, _files[req.path]);
			else
			{
				res.set_content("Must be authorized to see this page.", "text/html");
				res.status = 403;
			}

		}
		else
		{
			res.set_content("404 not found", "text/html");
			res.status = 404;
		}
	});
	_server->Post("/login-submit",[this](const httplib::Request &req, httplib::Response &res){

		try{
			Json::Value newUser;
			Json::Reader reader;
			if(!reader.parse(req.body, newUser))
			{
				std::cerr << "Problem parsing login request: " << req.body << std::endl;
				res.status = 400;
				res.set_content(R"({"text":"Request format incorrect","logged_in":false})", "application/json");
				return;
			}
			if(!newUser.isMember("username") || !newUser.isMember("password"))
			{
				res.status = 400;
				res.set_content(R"({"text":"Request format incorrect","logged_in":false})", "application/json");
				return;
			}


			std::string username = newUser["username"].asString();
			if(!_db.stringSafe(username))
			{
				res.set_content(R"({"text":"Invalid Characters","logged_in":false})", "application/json");
				return;
			}


			bool foundUser = false;
			std::string passHash;
			std::string salt;
			std::string userID;
			_db.rawSQLCall(
					"SELECT Logins.Password, Logins.Salt, Logins.UserID FROM Users INNER JOIN Logins ON Logins.UserID = Users.UserID WHERE lower(Users.Username)=lower('" +
					username + "');", [&](const std::vector<Database::sqlColumn>& columns)
					{
						foundUser = true;
						passHash = columns[0].value;
						salt = columns[1].value;
						userID = columns[2].value;
					});


			if(foundUser)
			{
				std::string hashedPassword = hashPassword(newUser["password"].asString(), salt);

				if(passHash == hashedPassword)
				{
					//Login stuff happen here
					std::string sessionID;
					while(sessionID.empty() || _sessions.count(sessionID))
					{
						sessionID = randHex(32);
					}



					SessionContext sc;
					sc.updateTimer();
					sc.userID = userID;
					sc.username = username;
					sc.permissions = _db.userPermissions(userID);
					_sessions.insert({sessionID, sc});

					setCookie("session_id", sessionID, res);
					res.set_content(R"({"text":"Login successful","logged_in":true})", "application/json");
					return;
				}
			}


			//Not login stuff happen here
			res.set_content(R"({"text":"Login fail","logged_in":false})", "application/json");
		}
		catch(const std::exception& e){
			std::cerr << "login submission error: " << e.what();
			res.set_content(R"({"text":"Internal error","logged_in":false})", "application/json");
			res.status = 500;
		}
	});
	_server->Post("/create-account-submit",[this](const httplib::Request &req, httplib::Response &res){
		try{
			Json::Value newUser;
			Json::Reader reader;
			if(!reader.parse(req.body, newUser))
			{
				std::cerr << "Problem parsing login request: " << req.body << std::endl;
				res.status = 400;
				res.set_content(R"({"text":"Request format incorrect","created":false})", "application/json");
				return;
			}
			if(!newUser.isMember("username") || !newUser.isMember("password") || !newUser.isMember("email"))
			{
				res.status = 400;
				res.set_content(R"({"text":"Request format incorrect","created":false})", "application/json");
				return;
			}
			std::string username = newUser["username"].asString();
			std::string email = newUser["email"].asString();
			std::string password = newUser["password"].asString();

			if(username == "")
			{
				res.set_content(R"({"text":"Must enter a username","created":false})", "application/json");
				return;
			}

			if(password.size() < 8)
			{
				res.set_content(R"({"text":"Password must be at least 8 characters long","created":false})", "application/json");
				return;
			}

			if(!_db.stringSafe(username))
			{
				res.set_content(R"({"text":"Invalid characters in username","created":false})", "application/json");
				return;
			}

			std::regex emailRegex(R"(^([a-zA-Z0-9_\-\.]+)@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.)|(([a-zA-Z0-9\-]+\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\]?)$)");
			if(!std::regex_match(email, emailRegex))
			{
				res.set_content(R"({"text":"Invalid email","created":false})", "application/json");
				return;
			}


			bool usernameTaken = false;
			_db.rawSQLCall("SELECT * FROM Users WHERE lower(Username)=lower('" + username + "');",
			               [&usernameTaken](const std::vector<Database::sqlColumn>& columns)
			               {
				               usernameTaken = true;
				               return 0;
			               });

			if(usernameTaken)
			{
				res.set_content(R"({"text":"Username Taken","created":false})", "application/json");
				return;
			}

			_db.rawSQLCall("INSERT INTO Users (Username, Email) VALUES ('" +
			               username + "', '" +
			               email +
			               "');", [&usernameTaken](const std::vector<Database::sqlColumn>& columns)
			               {
				               return 0;
			               });

			std::string salt = randHex(64);
			password = hashPassword(password, salt);

			std::string userID;
			_db.rawSQLCall("SELECT UserID FROM Users WHERE lower(Username)=lower('" + username + "');",
			               [&userID](const std::vector<Database::sqlColumn>& columns)
			               {
				               userID = columns[0].value;
				               return 0;
			               });

			_db.rawSQLCall("INSERT INTO Logins (UserID, Password, Salt) VALUES ('" +
			               userID + "', '" +
			               password + "', '" +
			               salt +
			               "');", [&usernameTaken](const std::vector<Database::sqlColumn>& columns)
			               {
				               return 0;
			               });


			res.set_content(R"({"text":"Created account","created":true})", "application/json");
		}
		catch(const std::exception& e){
			std::cerr << "user creation error: " << e.what();
			res.set_content(R"({"text":"Internal error","created":false})", "application/json");
			res.status = 500;
		}
	});

}

void AssetHttpServer::setUpAPICalls()
{
	_server->Get("/api/assets", [this](const httplib::Request &req, httplib::Response &res)//TODO: Paginate the data from this query, and add filters
	{
		try{
			std::string sessionID = getCookie("session_id", req);
			if(!authorizeSession(sessionID, {"create assets"}))
			{
				res.status = 403;
				res.set_content(R"({"text":"Not authorized for that action"})", "application/json");
				return;
			}

			std::vector<AssetData> assets = _db.listUserAssets(std::stoi(_sessions[sessionID].userID));
			Json::Value resJson;
			resJson["successful"] = true;
			for(auto& asset : assets)
			{
				Json::Value assetJson;
				assetJson["name"] = asset.name;
				assetJson["id"] = toHex(asset.id.id);
				assetJson["type"] = asset.type.string();
				resJson["assets"].append(assetJson);
			}
			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";
			res.set_content(Json::writeString(builder, resJson), "application/json");

		}
		catch(const std::exception& e){
			res.set_content(R"({"successful":false})", "application/json");
			std::cerr << "asset list error: " << e.what() << std::endl;
			res.status = 500;
		}

	});
	_server->Get("/api/assets/([a-fA-F0-9]+)", [this](const httplib::Request &req, httplib::Response &res)
	{
		try{
			std::string sessionID = getCookie("session_id", req);
			if(!authorizeSession(sessionID, {"create assets"}))
			{
				res.status = 403;
				res.set_content(R"({"text":"Not authorized for that action","successful":false})", "application/json");
				return;
			}
			std::cout << "User requesting asset: " << req.matches[1].str() << std::endl;
			AssetID assetID = AssetID("/" +req.matches[1].str());

			AssetPermission pem = _db.assetPermission(assetID.id, std::stoi(_sessions[sessionID].userID));
			if(pem.level() == AssetPermission::Level::none)
			{
				res.status = 403;
				res.set_content(R"({"text":"This asset does not exist, or you are not authorized to view it.","successful":false})", "application/json");
				return;
			}

			AssetData asset = _db.loadAssetData(assetID);
			Json::Value resJson;

			resJson["successful"] = true;
			resJson["name"] = asset.name;
			resJson["permission_level"] = (int)pem.level();
			resJson["type"] = asset.type.string();

			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";
			res.set_content(Json::writeString(builder, resJson), "application/json");

		}
		catch(const std::exception& e){
			res.set_content(R"({"successful":false})", "application/json");
			std::cerr << "asset list error: " << e.what();
			res.status = 500;
		}

	});
	_server->Get("/api/assets/get_name", [this](const httplib::Request &req, httplib::Response &res)
	{
		try{
			std::string sessionID = getCookie("session_id", req);
			if(!authorizeSession(sessionID, {"create assets"}))
			{
				res.status = 403;
				res.set_content(R"({"text":"Not authorized for that action","successful":false})", "application/json");
				return;
			}

			AssetID assetID = AssetID(req.get_header_value("name"));
			std::cout << "User requesting asset name: " << assetID.string() << std::endl;
			if(assetID.serverAddress != "native")
			{
				AssetPermission pem = _db.assetPermission(assetID.id, std::stoi(_sessions[sessionID].userID));
				if(pem.level() == AssetPermission::Level::none)
				{
					res.status = 403;
					res.set_content(R"({"text":"This asset does not exist, or you are not authorized to view it.","successful":false})", "application/json");
					return;
				}
			}

			Json::Value resJson;
			resJson["successful"] = true;
			resJson["name"] = assetID.string();
			if(assetID.serverAddress == "localhost" || assetID.serverAddress == "native")
				resJson["name"] = _am.getAssetName(assetID);

			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";
			res.set_content(Json::writeString(builder, resJson), "application/json");

		}
		catch(const std::exception& e){
			res.set_content(R"("successful":false)", "application/json");
			std::cerr << "asset list error: " << e.what();
			res.status = 500;
		}

	});
	_server->Get("/api/assets/([a-fA-F0-9]+)/data", [this](const httplib::Request &req, httplib::Response &res)
	{
		try{
			std::string sessionID = getCookie("session_id", req);
			if(!authorizeSession(sessionID, {"create assets"}))
			{
				res.status = 403;
				res.set_content(R"({"text":"Not authorized for that action","successful":false})", "application/json");
				return;
			}
			std::cout << "User requesting asset data: " << req.matches[1].str() << std::endl;
			AssetID assetID = AssetID("localhost/" +req.matches[1].str());

			AssetPermission pem = _db.assetPermission(assetID.id, std::stoi(_sessions[sessionID].userID));
			if(pem.level() == AssetPermission::Level::none)
			{
				res.status = 403;
				res.set_content(R"({"text":"This asset does not exist, or you are not authorized to view it.","successful":false})", "application/json");
				return;
			}

			Asset* asset = _am.getAsset<Asset>(assetID);
			assert(asset);
			Json::Value resJson;

			resJson["successful"] = true;
			resJson["type"] = asset->type.string();
			resJson["data"];
			if(asset->type.type() == AssetType::Type::assembly)
				resJson["data"] = Assembly::toJson((Assembly*)asset);

			Json::StreamWriterBuilder builder;
			builder["indentation"] = "";
			res.set_content(Json::writeString(builder, resJson), "application/json");
		}
		catch(const std::exception& e){
			res.set_content(R"("successful":false)", "application/json");
			std::cerr << "asset retrieval error: " << e.what();
			res.status = 500;
		}
	});
    _server->Post("/api/assets/create/gltf",[this](const httplib::Request &req, httplib::Response &res)
    {
        try{
            std::string sessionID = getCookie("session_id", req);
            if(authorizeSession(sessionID, {"create assets"}))
            {
                std::cout << "Extracting Data From GLTF" << std::endl;
                const httplib::MultipartFormData& assetDataField = req.get_file_value("assetData");
                std::cout << "AssetData: " << assetDataField.content << std::endl;
				// Convert to json
				Json::Value assetData;
	            Json::Reader reader;
	            if(!reader.parse( assetDataField.content, assetData))
	            {
		            std::cerr << "Problem parsing assetData: " << req.body << std::endl;
		            res.status = 400;
		            res.set_content(R"({"text":"Request format incorrect"})", "application/json");
		            return;
	            }

				const auto& file = req.get_file_value("file");

				gltfLoader loader;
				loader.loadGlbFromString(file.content);

	            Assembly assembly;
	            std::vector<MeshAsset*> meshes = AssetBuilder::extractMeshesFromGltf(loader);
				for(auto mesh : meshes)
				{
					std::cout << "Extracted mesh: " << mesh->name << "\n";

					AssetData ad(_db);
					ad.name = mesh->name;
					ad.id.serverAddress = _domain;
					ad.type.set(AssetType::Type::mesh);

					ad.save(); // Saving will generate an Asset ID
					mesh->id = ad.id;
					_fm.writeAsset(mesh);
					std::cout << "Created new asset with id: " << ad.id.string() << "\n";
					AssetPermission p = _db.assetPermission(mesh->id.id, std::stoi(_sessions[sessionID].userID));
					p.setLevel(AssetPermission::Level::owner);

					assembly.meshes.push_back(mesh->id);
				}
				try
				{
					assembly.data = AssetBuilder::extractNodes(loader);
				}
				catch(const std::exception& e){
					std::cerr << "asset upload error: " << e.what();
					res.status = 500;
				}
	            AssetData ad(_db);
				ad.name = assetData["name"].asString();
				AssetID assemblyID;
				assemblyID.serverAddress = _domain;
				ad.id = assemblyID;
				ad.type.set(AssetType::Type::assembly);
				ad.folderID = 0;
				ad.save();
				assembly.id = ad.id;
				assembly.name = ad.name;
				assembly.loadState = Asset::complete;

	            AssetPermission p = _db.assetPermission(ad.id.id, std::stoi(_sessions[sessionID].userID));
	            p.setLevel(AssetPermission::Level::owner);

				_fm.writeAsset(&assembly);
				std::cout << "Created assembly with id: " << ad.id.string() << std::endl;


				for(auto mesh : meshes)
					delete mesh; // We don't immediately need to use the mesh data after this, so store it in the file system and then free the memory;
				res.set_content(R"({"text":"Assembly created successfully","created":true})", "application/json");
            }
            else
            {
                res.status = 403;
                res.set_content(R"({"text":"Not authorized for that action","created":false})", "application/json");
            }
        }
        catch(const std::exception& e){
            std::cerr << "asset upload error: " << e.what();
            res.status = 500;
        }

    });
}

void AssetHttpServer::SessionContext::updateTimer()
{
	lastAction = std::chrono::system_clock::now();
}

bool AssetHttpServer::SessionContext::userAuthorized(serverFile& file)
{
	return file.authLevel == "public" || file.authLevel == "app" || permissions.count(file.authLevel);
}

bool AssetHttpServer::SessionContext::hasPermission(const std::string& permission)
{
	return permissions.count(permission);
}

#define WIN32_LEAN_AND_MEAN
#include <openssl/rand.h>
#include <openssl/crypto.h>

std::string AssetHttpServer::hashPassword(const std::string& password, const std::string& salt)
{
	size_t hashIterations = Config::json()["security"].get("hash_iterations", 10000).asLargestInt();
	unsigned char hash[SHA256_DIGEST_LENGTH];
	std::string output = password;
	for (int i = 0; i < hashIterations; ++i)
	{
		output += salt;
		SHA256_CTX sha256;
		SHA256_Init(&sha256);
		SHA256_Update(&sha256, output.c_str(), output.size());
		SHA256_Final(hash, &sha256);
		std::stringstream ss;
		for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
		{
			ss << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
		}
		output = ss.str();
	}
	return output;
}

std::string AssetHttpServer::randHex(size_t length)
{
	uint8_t* buffer = new uint8_t[length];
	RAND_bytes(buffer, length);
	std::stringstream output;
	for(int i = 0; i < SHA256_DIGEST_LENGTH; i++)
	{
		output << std::hex << std::setw(2) << std::setfill('0') << (int)buffer[i];
	}
	return output.str();
}

bool AssetHttpServer::authorizeSession(const std::string& sessionID, std::vector<std::string> requiredAuths)
{
	if(!_sessions.count(sessionID))
		return false;
	for (auto& auth : requiredAuths)
	{
		if(!_sessions[sessionID].hasPermission(auth))
			return false;
	}
	return true;
}


