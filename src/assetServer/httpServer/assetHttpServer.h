#include <http/HTTPServer.h>

class AssetHttpServer : public HTTPServer{
	Database& _db;
	struct SessionContext
	{
		std::chrono::time_point<std::chrono::system_clock> lastAction;
		std::string userID;
		std::unordered_set<std::string> permissions;
		void updateTimer();
		bool userAuthorized(serverFile& file);
		bool hasPermission(const std::string& permission);
	};
	std::unordered_map<std::string, SessionContext> _sessions;

	std::string randHex(size_t length);
	std::string hashPassword(const std::string& password, const std::string& salt);

	void setUpListeners();
    void setUpAPICalls();
	void processAsset(const std::string filename, const std::string data);
public:
	AssetHttpServer(const std::string& domain, bool useHttps, Database& db);

};