#include "assetHttpServer.h"

AssetHttpServer::AssetHttpServer(const std::string& domain, bool useHttps, Database& db): HTTPServer(domain, useHttps), _db(db)
{
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
				res.set_content("{\"text\":\"Request format incorrect\",\"logged_in\":false}", "application/json");
				return;
			}
			if(!newUser.isMember("username") || !newUser.isMember("password"))
			{
				res.status = 400;
				res.set_content("{\"text\":\"Request format incorrect\",\"logged_in\":false}", "application/json");
				return;
			}


			std::string username = newUser["username"].asString();
			if(!_db.stringSafe(username))
			{
				res.set_content("{\"text\":\"Invalid Characters\",\"logged_in\":false}", "application/json");
				return;
			}

			for (int i = 0; i < username.size(); ++i)
				username[i] = std::tolower(username[i]);


			bool foundUser = false;
			std::string passHash;
			std::string salt;
			std::string userID;
			_db.rawSQLCall(
					"SELECT Logins.Password, Logins.Salt, Logins.UserID FROM Users INNER JOIN Logins ON Logins.UserID = Users.UserID WHERE lower(Users.Username)='" +
					username + "';", [&](const std::vector<Database::sqlColumn>& columns)
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
					sc.permissions = _db.userPermissions(userID);
					_sessions.insert({sessionID, sc});

					setCookie("session_id", sessionID, res);
					res.set_content("{\"text\":\"Login successful\",\"logged_in\":true}", "application/json");
					return;
				}
			}


			//Not login stuff happen here
			res.set_content("{\"text\":\"Login fail\",\"logged_in\":false}", "application/json");
		}
		catch(const std::exception& e){
			std::cerr << "login submission error: " << e.what();
			res.set_content("{\"text\":\"Internal error\",\"logged_in\":false}", "application/json");
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
				res.set_content("{\"text\":\"Request format incorrect\",\"created\":false}", "application/json");
				return;
			}
			if(!newUser.isMember("username") || !newUser.isMember("password") || !newUser.isMember("email"))
			{
				res.status = 400;
				res.set_content("{\"text\":\"Request format incorrect\",\"created\":false}", "application/json");
				return;
			}
			std::string username = newUser["username"].asString();
			std::string email = newUser["email"].asString();
			std::string password = newUser["password"].asString();

			if(username == "")
			{
				res.set_content("{\"text\":\"Must enter a username\",\"created\":false}", "application/json");
				return;
			}

			if(password.size() < 8)
			{
				res.set_content("{\"text\":\"Password must be at least 8 characters long\",\"created\":false}", "application/json");
				return;
			}

			if(!_db.stringSafe(username))
			{
				res.set_content("{\"text\":\"Invalid characters in username\",\"created\":false}", "application/json");
				return;
			}

			std::regex emailRegex("^([a-zA-Z0-9_\\-\\.]+)@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.)|(([a-zA-Z0-9\\-]+\\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\\]?)$");
			if(!std::regex_match(email, emailRegex))
			{
				res.set_content("{\"text\":\"Invalid email\",\"created\":false}", "application/json");
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
				res.set_content("{\"text\":\"Username Taken\",\"created\":false}", "application/json");
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


			res.set_content("{\"text\":\"Created account\",\"created\":true}", "application/json");
		}
		catch(const std::exception& e){
			std::cerr << "user creation error: " << e.what();
			res.set_content("{\"text\":\"Internal error\",\"created\":false}", "application/json");
			res.status = 500;
		}
	});
	_server->Post("/upload-asset",[this](const httplib::Request &req, httplib::Response &res)
	{
		try{
			std::string sessionID = getCookie("session_id", req);
			if(_sessions.count(sessionID) && _sessions[sessionID].hasPermission("upload assets"))
			{
				std::cout << "Asset uploaded" << std::endl;
				bool ret = req.has_file("asset");
				const auto& file = req.get_file_value("asset");
				std::cout << "Filetype: " << file.content_type << std::endl;
				std::cout << "Filename: " << file.filename << std::endl;
				std::string fileData = file.content;
				std::string filename = file.filename;
				ThreadPool::enqueue([fileData, filename, this](){
					processAsset(filename, fileD ata);
				});



			}
			else
			{
				res.status = 403;
				res.set_content("Not authorized for that action", "text/plain");
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