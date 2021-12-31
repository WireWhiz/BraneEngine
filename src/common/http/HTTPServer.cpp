//
// Created by wirewhiz on 12/29/21.
//

#include "HTTPServer.h"
#include "../../../external/libs/include/acme-lw.h"
#include <time.h>

HTTPServer::HTTPServer(const std::string& domain, bool useHttps)
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
                            "    <p>Please use https <a href=\"https://"+ _domain + request.path+ "\">home</a>.</p>\n"
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
    std::cout << "Scanning files\n";
    _server->Get("/(.*)", [this](const httplib::Request &request, httplib::Response &res){
        std::cout << "Insulting user\n";
        std::cout << "Request was: " << request.path << std::endl;
        res.set_content("Your mom", "text/insult");
    });


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


