//
// Created by wirewhiz on 12/29/21.
//

#include "HTTPServer.h"
#include <openssl/rand.h>
#include <openssl/crypto.h>
#include <ctime>
#include <assets/types/loginDataAsset.h>
#include <sqlite/sqlite3.h>

HTTPServer::HTTPServer(const std::string& domain, FileManager& fm, Database& db,  bool useHttps) : _template("pages/template.html"), _fm(fm), _db(db)
{
    _domain = domain;
	_useHttps = useHttps;

    if(useHttps)
    {
        if(Config::json()["network"]["auto_renew_key"].asBool())
        {

            std::string cert;
            std::ifstream certFile(Config::json()["network"]["ssl_cert"].asString(), std::ios::binary);
            if(!certFile.is_open())
                throw;
            std::stringstream buffer;
            buffer << certFile.rdbuf();
            cert = buffer.str();
            certFile.close();

            acme_lw::Certificate certificate;
            certificate.fullchain = cert;
            time_t currentTime;
            time(&currentTime);
            std::cout << (difftime(certificate.getExpiry(), currentTime) / 86400) << " days until ssl cert expires." << std::endl;
            if(difftime(certificate.getExpiry(), currentTime) / 86400 < 28)
            {
                _server = std::make_unique<httplib::Server>();
                _mainCtx = std::thread([this](){
                    _server->listen(_domain.c_str(), 80);
                });
                renewCert();
                _server->stop();
                _mainCtx.join();
            }

        }

        _redirectServer = std::make_unique<httplib::Server>();
        _redirectServer->Get("/(.*)", [this](const httplib::Request &request, httplib::Response & res){
            res.set_content("<!DOCTYPE html>\n"
                            "<html>\n"
                            "  <head>\n"
                            "    <meta http-equiv=\"refresh\" content=\"0; url='https://" + _domain + request.path+ "'\" />\n"
                            "  </head>\n"
                            "  <body>\n"
                            "    <p><a href=\\\"https://\"+ _domain + request.path+ \"\\\">Please use https</a></p>\n"
                            "  </body>\n"
                            "</html>", "text/html");
        });
        _redirectCtx = std::thread([this](){
            _redirectServer->listen(_domain.c_str(), 80);
        });

        _server = std::make_unique<httplib::SSLServer>(Config::json()["network"]["ssl_cert"].asCString(), Config::json()["network"]["private_key"].asCString());
        _mainCtx = std::thread([this](){
            _server->listen(_domain.c_str(), 443);
        });


    }
    else
    {
        _server = std::make_unique<httplib::Server>();
        _mainCtx = std::thread([this](){
            _server->listen(_domain.c_str(), 80);
        });

    }

	_server->Get("/app/(.*)", [this](const httplib::Request &req, httplib::Response &res){
		if(_sessions.count(getCookie("session_id", req)))
		{
			res.set_content("Access: granted", "text/html");
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
			std::string username = req.get_header_value("username");

			if(!_db.stringSafe(username))
			{
				res.set_content("Invalid Characters", "text/plain");
				return;
			}

			for (int i = 0; i < username.size(); ++i)
				username[i] = std::tolower(username[i]);


			bool foundUser = false;
			std::string passHash;
			std::string salt;
			std::string userID;
			_db.sqlCall("SELECT Logins.Password, Logins.Salt, Logins.UserID FROM Users INNER JOIN Logins ON Logins.UserID = Users.UserID WHERE lower(Users.Username)='" + username + "';", [&](const std::vector<Database::sqlColumn>& columns){
				foundUser = true;
				passHash = columns[0].value;
				salt = columns[1].value;
				userID = columns[2].value;
				return 0;
			});


			if(foundUser)
			{
				std::string hashedPassword = hashPassword(req.get_header_value("password"), salt);

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
					_sessions.insert({sessionID, sc});

					setCookie("session_id", sessionID, res);
					res.set_content("Login successful", "text/plain");
					return;
				}
			}


			//Not login stuff happen here
			res.set_content("Login fail", "text/plain");
		}
		catch(const std::exception& e){
			std::cerr << "login submission error: " << e.what();
		}
	});
	_server->Post("/create-account-submit",[this](const httplib::Request &req, httplib::Response &res){
		try{
			std::string username = req.get_header_value("username");
			std::string email = req.get_header_value("email");

			if(username == "")
			{
				res.set_content("Must enter a username", "text/plain");
				return;
			}

			if(req.get_header_value("password").size() < 8)
			{
				res.set_content("Password must be at least 8 characters long", "text/plain");
				return;
			}

			if(!_db.stringSafe(username) || !_db.stringSafe(email))
			{
				res.set_content("Invalid Characters", "text/plain");
				return;
			}

			std::regex emailRegex("^([a-zA-Z0-9_\\-\\.]+)@((\\[[0-9]{1,3}\\.[0-9]{1,3}\\.[0-9]{1,3}\\.)|(([a-zA-Z0-9\\-]+\\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\\]?)$");
			if(!std::regex_match(email, emailRegex))
			{
				res.set_content("Invalid email", "text/plain");
				return;
			}

			for (int i = 0; i < username.size(); ++i)
			{
				username[i] = std::tolower(username[i]);
			}


			bool usernameTaken = false;
			_db.sqlCall("SELECT * FROM Users WHERE lower(Username)='" + username + "';", [&usernameTaken](const std::vector<Database::sqlColumn>& columns){
				usernameTaken = true;
				return 0;
			});

			if(usernameTaken)
			{
				res.set_content("Username Taken", "text/plain");
				return;
			}

			_db.sqlCall("INSERT INTO Users (Username, Email) VALUES ('" +
						req.get_header_value("username") + "', '" +
						req.get_header_value("email") +
						"');", [&usernameTaken](const std::vector<Database::sqlColumn>& columns){
				return 0;
			});

			std::string salt = randHex(64);
			std::string password = hashPassword(req.get_header_value("password"), salt);

			std::string userID;
			_db.sqlCall("SELECT UserID FROM Users WHERE lower(Username)='" + username + "';", [&userID](const std::vector<Database::sqlColumn>& columns){
				userID = columns[0].value;
				return 0;
			});

			_db.sqlCall("INSERT INTO Logins (UserID, Password, Salt) VALUES ('" +
			            userID + "', '" +
			            password + "', '" +
			            salt +
			            "');", [&usernameTaken](const std::vector<Database::sqlColumn>& columns){
				return 0;
			});


			//Login stuff happen here
			res.set_content("Created account", "text/plain");
		}
		catch(const std::exception& e){
			std::cerr << "user creation error: " << e.what();
		}
	});
}

HTTPServer::~HTTPServer()
{
    if(_redirectServer)
        _redirectServer->stop();
    if(_redirectCtx.joinable())
        _redirectCtx.join();

    if(_server)
        _server->stop();
    if(_mainCtx.joinable())
        _mainCtx.join();
}

void HTTPServer::scanFiles()
{
    std::cout << "Refreshing webpages\n";
    std::filesystem::path pagesDir{Config::json()["network"]["web_file_dir"].asString()};
    _files.clear();
    for(auto const& path: std::filesystem::directory_iterator{pagesDir}) //Iterate over every path in the pages directory
    {
        if(!path.is_directory()) // We only want to step into folders
            continue;
        std::cout << "Found folder with pages: " << path << std::endl;
        for(auto const& file: std::filesystem::directory_iterator{path}) //Iterate over every path in the pages directory
        {
            if(!file.is_regular_file()) // Only serve files for now
                continue;

            std::string httpPath;
            if(file.path().extension() != ".html")
                httpPath = file.path().filename().string();
            else if(file.path().stem().string() != "index")
            {
                httpPath = file.path().stem().string();

            }
            std::cout << "added path: " + httpPath << std::endl;
            _files.insert({"/" + httpPath ,serverFile{file.path(), file.path().parent_path().stem().string()}});

        }

    }



}

void HTTPServer::addResponse(const std::string &url, const std::function<void(const httplib::Request &, httplib::Response &)> &callback)
{
    assert(_server);
    if(_server)
        _server->Get(url, callback);
}

HTTPServer* HTTPServer::_renewInstance = nullptr;
void HTTPServer::renewCert()
{
    try
    {
        std::string accountKey;
        std::ifstream keyFile(Config::json()["network"]["account_key"].asString(), std::ios::binary);
        if (!keyFile.is_open())
            throw;
        std::stringstream keyBuffer;
        keyBuffer << keyFile.rdbuf();
        accountKey = keyBuffer.str();
        keyFile.close();

        _renewInstance = this;

        acme_lw::AcmeClient::init();
        acme_lw::AcmeClient acmeClient(accountKey);
        acme_lw::Certificate certificate = acmeClient.issueCertificate({std::string("branevr.com")}, httpChallenge);
        acme_lw::AcmeClient::teardown();

        std::ofstream cert(Config::json()["network"]["ssl_cert"].asString(), std::ios::binary);
        assert(cert.is_open());
        cert << certificate.fullchain;
        cert.close();

        std::ofstream key(Config::json()["network"]["private_key"].asString(), std::ios::binary);
        assert(key.is_open());
        key << certificate.privkey;
        key.close();
    }catch(const std::exception& e){
        std::cerr << "Failed to renew ssl cert: " << e.what() << std::endl;
    }
}

void HTTPServer::httpChallenge(const std::string &domainName, const std::string &url, const std::string &keyAuthorization)
{
    size_t domLength = _renewInstance->_domain.size() + 7; //Length of "http://domain.com"
    _renewInstance->_server->Get( url.substr(domLength, url.size() - domLength), [keyAuthorization](const httplib::Request &, httplib::Response &res) {
        res.set_content(keyAuthorization, "text/plain");
    });
}

void HTTPServer::serveFile(const httplib::Request &req, httplib::Response &res, HTTPServer::serverFile &file)
{
    std::ifstream f(file.path, std::ios::binary | std::ios::ate);
    std::string content;
    content.resize(f.tellg());
    f.seekg(0);
    f.read(content.data(), content.size());

    std::string fileType = getFileType(file.path.extension().string());
    SessionContext sc{};
    if(fileType == "text/html" && file.path.string().find("public") != std::string::npos)
        content = _template.format(content, sc);

    res.set_content(content, fileType.c_str());
}

std::map<std::string, std::string> HTTPServer::_mimetypes = {
        { ".css", "text/css"},
        { ".csv", "text/csv"},
        { ".txt", "text/plain"},
        { ".vtt", "text/vtt"},
        { ".htm", "text/html"},
        { ".html", "text/html"},

        { ".apng", "image/apng"},
        { ".avif", "image/avif"},
        { ".bmp", "image/bmp"},
        { ".gif", "image/gif"},
        { ".png", "image/png"},
        { ".svg", "image/svg+xml"},
        { ".webp", "image/webp"},
        { ".ico", "image/x-icon"},
        { ".tif", "image/tiff"},
        { ".tiff", "image/tiff"},
        { ".jpg", "image/jpeg"},
        { ".jpeg", "image/jpeg"},

        { ".mp4", "video/mp4"},
        { ".mpeg", "video/mpeg"},
        { ".webm", "video/webm"},

        { ".mp3", "audio/mp3"},
        { ".mpga", "audio/mpeg"},
        { ".weba", "audio/webm"},
        { ".wav", "audio/wave"},

        { ".otf", "font/otf"},
        { ".ttf", "font/ttf"},
        { ".woff", "font/woff"},
        { ".woff2", "font/woff2"},

        { ".7z", "application/x-7z-compressed"},
        { ".atom", "application/atom+xml"},
        { ".pdf", "application/pdf"},
        { ".js", "application/javascript"},
        { ".mjs", "application/javascript"},
        { ".json", "application/json"},
        { ".rss", "application/rss+xml"},
        { ".tar", "application/x-tar"},
        { ".xht", "application/xhtml+xml"},
        { ".xhtml", "application/xhtml+xml"},
        { ".xslt", "application/xslt+xml"},
        { ".xml", "application/xml"},
        { ".gz", "application/gzip"},
        { ".zip", "application/zip"},
        { ".wasm", "application/wasm"}
};
std::string HTTPServer::getFileType(const std::string &extension) const
{
    if(_mimetypes.count(extension))
        return _mimetypes[extension];
    return "text/plain";
}

void HTTPServer::setCookie(const std::string &key, const std::string &value, httplib::Response &res) const
{
    res.set_header("Set-Cookie",key + "=" + value);
}

std::string HTTPServer::getCookie(const std::string &key, const httplib::Request &req) const
{
    if(!req.has_header("Cookie"))
        return "";
    std::string cookies = req.get_header_value("Cookie");
    size_t cookieIndex = cookies.find(key);
    if(cookieIndex == std::string::npos)
        return "";

    cookieIndex += key.length() + 1;
    size_t cookieEnd = cookies.find(";", cookieIndex);
    if(cookieEnd == std::string::npos)
        cookieEnd = cookies.size();

    return cookies.substr(cookieIndex, cookieEnd - cookieIndex);
}

std::string HTTPServer::hashPassword(const std::string& password, const std::string& salt)
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

std::string HTTPServer::randHex(size_t length)
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

std::string HTTPServer::PageTemplate::format(const std::string &content, const HTTPServer::SessionContext &ctx)
{
    std::string output;
    output.reserve(content.size() + sections[0].size() + sections[1].size()); // Hopefully this makes it so that there is only one memory allocation from this fuction
    output += sections[0];
    output += content;
    output += sections[1];
    return output;
}


HTTPServer::PageTemplate::PageTemplate(std::filesystem::path templateFile)
{
    std::ifstream f(templateFile, std::ios::binary | std::ios::ate);
    if(!f.is_open())
    {
        std::cerr << "Could not open required file: " << templateFile << std::endl;
        throw std::runtime_error("Could not open required file: " + templateFile.generic_string());
    }
    std::string temp;
    temp.resize(f.tellg());
    f.seekg(0);
    f.read(temp.data(), temp.size());
    f.close();

    size_t contentIndex = temp.find("[Content]");
    if(contentIndex == std::string::npos)
    {
        std::cerr << "Html template has no [Content] marker" << std::endl;
        throw std::runtime_error("Html template has no [Content] marker");
    }

    sections.resize(2);
    sections[0] = temp.substr(0, contentIndex);
    sections[1] = temp.substr(contentIndex + 9, temp.size() - (contentIndex + 9));
}

void HTTPServer::SessionContext::updateTimer()
{
	lastAction = std::chrono::system_clock::now();
}

bool HTTPServer::SessionContext::userAuthorized(serverFile& file)
{
	return file.authLevel == "public" || file.authLevel == "app" || permissions.count(file.authLevel);
}
