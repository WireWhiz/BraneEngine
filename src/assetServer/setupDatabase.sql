CREATE TABLE "Logins" (
	"UserID"	INTEGER NOT NULL UNIQUE,
	"Password"	varchar(64) NOT NULL,
	"Salt"	varchar(64) NOT NULL,
	PRIMARY KEY("UserID")
);

CREATE TABLE "Users" (
	"UserID"	INTEGER NOT NULL,
	"Username"	varchar(16) NOT NULL,
	"Email"	varchar(32) NOT NULL,
	PRIMARY KEY("UserID" AUTOINCREMENT)
);