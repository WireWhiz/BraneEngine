//
// Created by wirewhiz on 12/29/21.
//

#ifndef BRANEENGINE_HTTPSERVER_H
#define BRANEENGINE_HTTPSERVER_H

#define CPPHTTPLIB_OPENSSL_SUPPORT
#define WIN32_LEAN_AND_MEAN
#include <httpLib/httpLib.h>
#include <acme-lw.h>
#include <thread>
#include <memory>
#include <config/config.h>
#include <filesystem>
#include <fileManager/fileManager.h>
#include <database/Database.h>
#include <regex>
#include <chrono>

class HTTPServer
{
private:
	FileManager& _fm;
	Database& _db;
    bool _useHttps;
    std::unique_ptr<httplib::Server> _redirectServer;
    std::unique_ptr<httplib::Server> _server;
    std::thread _redirectCtx;
    std::thread _mainCtx;

    std::string _domain;

    static HTTPServer* _renewInstance; // Man this code is sketch
    void renewCert();
    static void httpChallenge(const std::string& domainName,
                              const std::string& url,
                              const std::string& keyAuthorization);

    static std::map<std::string, std::string> _mimetypes;
    struct serverFile
    {
        std::filesystem::path path;
        std::string authLevel;
    };

    std::unordered_map<std::string, serverFile> _files;

    void serveFile(const httplib::Request &req, httplib::Response &res, serverFile& file);
    std::string getFileType(const std::string& extension) const;

    void setCookie(const std::string& key, const std::string& value, httplib::Response& res) const;
    std::string getCookie(const std::string& key, const httplib::Request& req) const;

    struct SessionContext
    {
		std::chrono::time_point<std::chrono::system_clock> lastAction;
		std::string userID;
		std::unordered_set<std::string> permissions;
		void updateTimer();
		bool userAuthorized(serverFile& file);
    };
	std::unordered_map<std::string, SessionContext> _sessions;

    class PageTemplate
    {
        std::filesystem::path templateFile;
        std::vector<std::string> sections;
    public:
        PageTemplate(std::filesystem::path templateFile);
        std::string format(const std::string& content, const SessionContext& ctx);
    };

    PageTemplate _template;

	std::string randHex(size_t length);
	std::string hashPassword(const std::string& password, const std::string& salt);
public:
    HTTPServer(const std::string& domain, FileManager& fm, Database& db, bool useHttps);
    ~HTTPServer();

    void addResponse(const std::string& url, const std::function<void(const httplib::Request &, httplib::Response &res)>& callback); //For use with things like ACME challenge requests
    void scanFiles();
};


#endif //BRANEENGINE_HTTPSERVER_H
