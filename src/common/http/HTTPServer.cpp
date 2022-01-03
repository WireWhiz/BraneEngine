//
// Created by wirewhiz on 12/29/21.
//

#include "HTTPServer.h"
#include "../../../external/libs/include/acme-lw.h"
#include <time.h>

HTTPServer::HTTPServer(const std::string& domain, bool useHttps) : _template("pages/template.html")
{
    _domain = domain;

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
    _server->Get("/(.*)", [this](const httplib::Request &req, httplib::Response &res){
        std::cout << "Request: " << req.path <<std::endl;
        if(_files.count(req.path) > 0)
        {
            serveFile(req, res, _files[req.path]);
            setCookie("session_id", "69420", res);
            setCookie("second_test_cookie", "nice", res);
        }
        else
        {
            res.set_content("404 not found", "text/html");
            res.status = 404;
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
    std::filesystem::path pagesDir{"pages"};
    _files.clear();
    for(auto const& path: std::filesystem::directory_iterator{pagesDir}) //Iterate over every path in the pages directory
    {
        if(!path.is_directory()) // We only want to step into folders
            continue;
        std::cout << "Found folder: " << path << std::endl;
        for(auto const& file: std::filesystem::directory_iterator{path}) //Iterate over every path in the pages directory
        {
            std::cout << "searching dir: " << file.path() << std::endl;
            if(!file.is_regular_file()) // Only serve files for now
                continue;
            std::cout << "found file: " << file.path().stem() << " ext: " << file.path().extension()<< " In parent folder: " << file.path().parent_path().stem() << std::endl;

            std::string httpPath;
            if(file.path().extension() != ".html")
                httpPath = file.path().filename();
            else if(file.path().stem() != "index")
            {
                httpPath = file.path().stem();

            }
            std::cout << "added path: " + httpPath << std::endl;
            _files.insert({"/" + httpPath ,serverFile{file.path(), file.path().parent_path().stem()}});

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

    std::string fileType = getFileType(file.path.extension());
    SessionContext sc{};
    if(fileType == "text/html")
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
