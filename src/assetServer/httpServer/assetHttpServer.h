#include <http/HTTPServer.h>

class AssetHttpServer : public HTTPServer{
	Database& _db;
	FileManager _fm;
	struct SessionContext
	{
		std::chrono::time_point<std::chrono::system_clock> lastAction;
		std::string username;
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
	bool authorizeSession(const std::string& sessionID, std::vector<std::string> requiredAuths);
public:
	AssetHttpServer(const std::string& domain, bool useHttps, Database& db, FileManager& fm);

};