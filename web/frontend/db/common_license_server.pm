package common_license_server;

use lib '/home/pyxis/scripts/';
use db_tools;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(createLSTables dropLSTables);

sub createLSTables {
  $dbh=$_[0];
  
  # Pipeline Tables
  $query="CREATE TABLE IF NOT EXISTS PipelineDefinitions (ProcRef VARCHAR(54) NOT NULL, Definition LONGTEXT, PRIMARY KEY(ProcRef), State VARCHAR(10) DEFAULT 'Active')";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS PipelineMetaData (ProcRef VARCHAR(54) NOT NULL, DataSize BIGINT NOT NULL, User VARCHAR(255) NOT NULL, Name VARCHAR(255) NOT NULL, Description TEXT NOT NULL, Category VARCHAR(255), ImageUrl Text, Created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY(ProcRef), FOREIGN KEY (ProcRef) REFERENCES PipelineDefinitions(ProcRef) ON DELETE CASCADE ON UPDATE CASCADE)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS PipelineTags (id BIGINT NOT NULL AUTO_INCREMENT, ProcRef VARCHAR(54) NOT NULL, Tag VARCHAR(255) NOT NULL, PRIMARY KEY (id), FOREIGN KEY (ProcRef) REFERENCES PipelineDefinitions(ProcRef) ON DELETE CASCADE ON UPDATE CASCADE)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS Servers (NodeID VARCHAR(36) NOT NULL, Name VARCHAR(255), FreeDiskSpace DOUBLE NOT NULL, LastPing TIMESTAMP DEFAULT CURRENT_TIMESTAMP, PRIMARY KEY(NodeID))";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS PipelineServers (id BIGINT NOT NULL AUTO_INCREMENT, ProcRef VARCHAR(54) NOT NULL, NodeID VARCHAR(36) NOT NULL, StatusCode enum('Initializing','Downloading','Processing','Publishing','Removing','Published') NOT NULL, OperationStatus Text, PRIMARY KEY(id), FOREIGN KEY (ProcRef) REFERENCES PipelineDefinitions(ProcRef), FOREIGN KEY(NodeID) REFERENCES Servers(NodeID) ON DELETE CASCADE ON UPDATE CASCADE)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS PipelinePending (id CHAR(36) NOT NULL, Created TIMESTAMP DEFAULT CURRENT_TIMESTAMP, Payload LONGTEXT NOT NULL, PRIMARY KEY(id))";
  $sqlQuery=executeQuery($dbh,$query);

  # Channel Tables
  $query="CREATE TABLE IF NOT EXISTS ChannelDefinitions (ChannelID CHAR(36) NOT NULL, Viewpoint LONGTEXT, PRIMARY KEY(ChannelID))";
  $sqlQuery=executeQuery($dbh,$query);
  
  $query="CREATE TABLE IF NOT EXISTS ChannelPipelines (id BIGINT NOT NULL AUTO_INCREMENT, ChannelID CHAR(36) NOT NULL, ProcRef VARCHAR(54) NOT NULL, PRIMARY KEY(id), FOREIGN KEY (ProcRef) REFERENCES PipelineDefinitions(ProcRef), FOREIGN KEY(ChannelID) REFERENCES ChannelDefinitions(ChannelID) ON DELETE CASCADE ON UPDATE CASCADE)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS ChannelPipelineTags (id BIGINT NOT NULL AUTO_INCREMENT, ChannelPipelinesID BIGINT NOT NULL, Tag VARCHAR(255) NOT NULL, PRIMARY KEY(id), FOREIGN KEY(ChannelPipelinesID) REFERENCES ChannelPipelines(id) ON DELETE CASCADE ON UPDATE CASCADE)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS ChannelMetaData (ChannelID CHAR(36) NOT NULL, User VARCHAR(255) NOT NULL, Title VARCHAR(255) NOT NULL, Description TEXT NOT NULL, Category VARCHAR(255) DEFAULT NULL, Updated TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP, Created TIMESTAMP NULL, PRIMARY KEY(ChannelID), FOREIGN KEY (ChannelID) REFERENCES ChannelDefinitions(ChannelID) ON DELETE CASCADE)";
  $sqlQuery=executeQuery($dbh,$query);

  # Triggers
  # MySQL does not allow mutliple default timestamp columns in a single table
  $query="CREATE TRIGGER server_heard_time_on_update BEFORE UPDATE ON Servers FOR EACH ROW SET NEW.LastPing = now()";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TRIGGER channel_create_time_on_insert BEFORE INSERT ON ChannelMetaData FOR EACH ROW SET NEW.Created = now()";
  $sqlQuery=executeQuery($dbh,$query);
 
  $query="CREATE TRIGGER channel_update_time_on_update BEFORE UPDATE ON ChannelMetaData FOR EACH ROW SET NEW.Updated = now()";
  $sqlQuery=executeQuery($dbh,$query);

  # Views
  $query="CREATE VIEW PipelineUsable AS SELECT ProcRef, TRUE AS Usable FROM PipelineServers WHERE StatusCode = 'Published' GROUP BY ProcRef";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE VIEW PipelineInfo AS SELECT PipelineDefinitions.ProcRef, Definition, State, DataSize, User, Name, Description, Category, ImageUrl, Created, Usable FROM PipelineDefinitions LEFT JOIN PipelineMetaData ON PipelineDefinitions.ProcRef = PipelineMetaData.ProcRef LEFT JOIN PipelineUsable ON PipelineDefinitions.ProcRef = PipelineUsable.ProcRef";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE VIEW PipelineInfoNoDefinition AS SELECT PipelineDefinitions.ProcRef, State, DataSize, User, Name, Description, Category, ImageUrl, Created, Usable FROM PipelineDefinitions LEFT JOIN PipelineMetaData ON PipelineDefinitions.ProcRef = PipelineMetaData.ProcRef LEFT JOIN PipelineUsable ON PipelineDefinitions.ProcRef = PipelineUsable.ProcRef";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE VIEW PipelineStatuses AS SELECT PipelineMetaData.ProcRef, User, IF(NodeID IS NULL,'',NodeID) AS NodeID, StatusCode, OperationStatus FROM PipelineMetaData LEFT JOIN PipelineServers ON PipelineMetaData.ProcRef = PipelineServers.ProcRef";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE VIEW ChannelTaggedPipelines AS SELECT ChannelID, ProcRef, IF(Tag IS NULL,'',TAG) AS Tag FROM ChannelPipelines LEFT JOIN ChannelPipelineTags ON ChannelPipelines.id = ChannelPipelineTags.ChannelPipelinesID";
  $sqlQuery=executeQuery($dbh,$query);

  # Authentication
  $query="CREATE TABLE IF NOT EXISTS `UserProfile` (`UserId` int(11) NOT NULL AUTO_INCREMENT, `UserName` longtext NOT NULL, PRIMARY KEY (`UserId`), UNIQUE KEY `UserId` (`UserId`))";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS `webpages_Membership` (`UserId` int(11) NOT NULL,`CreateDate` datetime DEFAULT NULL, `ConfirmationToken` varchar(128) CHARACTER SET utf8 DEFAULT NULL, `IsConfirmed` tinyint(1) DEFAULT NULL, `LastPasswordFailureDate` datetime DEFAULT NULL, `PasswordFailuresSinceLastSuccess` int(11) NOT NULL, `Password` varchar(128) CHARACTER SET utf8 DEFAULT NULL, `PasswordChangedDate` datetime DEFAULT NULL, `PasswordSalt` varchar(128) CHARACTER SET utf8 DEFAULT NULL, `PasswordVerificationToken` varchar(128) CHARACTER SET utf8 DEFAULT NULL, `PasswordVerificationTokenExpirationDate` datetime DEFAULT NULL, PRIMARY KEY (`UserId`), CONSTRAINT `Membership_UserProfile` FOREIGN KEY (`UserId`) REFERENCES `UserProfile` (`UserId`) ON DELETE NO ACTION ON UPDATE NO ACTION)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS `webpages_OAuthMembership` (`Provider` varchar(30) CHARACTER SET utf8 NOT NULL, `ProviderUserId` varchar(100) CHARACTER SET utf8 NOT NULL, `UserId` int(11) NOT NULL, PRIMARY KEY (`Provider`,`ProviderUserId`), KEY `UserId` (`UserId`), CONSTRAINT `OAuthMembership_UserProfile` FOREIGN KEY (`UserId`) REFERENCES `UserProfile` (`UserId`) ON DELETE CASCADE ON UPDATE NO ACTION)";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS `webpages_OAuthToken` (`Token` varchar(100) CHARACTER SET utf8 NOT NULL, `Secret` varchar(100) CHARACTER SET utf8 DEFAULT NULL, PRIMARY KEY (`Token`))";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS `webpages_Roles` (`RoleId` int(11) NOT NULL AUTO_INCREMENT, `RoleName` varchar(256) CHARACTER SET utf8 DEFAULT NULL, PRIMARY KEY (`RoleId`), UNIQUE KEY `RoleId` (`RoleId`))";
  $sqlQuery=executeQuery($dbh,$query);

  $query="CREATE TABLE IF NOT EXISTS `webpages_UsersInRoles` (`UserId` int(11) NOT NULL, `RoleId` int(11) NOT NULL, PRIMARY KEY (`UserId`,`RoleId`), KEY `UsersInRoles_Role` (`RoleId`), CONSTRAINT `UsersInRoles_Role` FOREIGN KEY (`RoleId`) REFERENCES `webpages_Roles` (`RoleId`) ON DELETE CASCADE ON UPDATE NO ACTION, CONSTRAINT `UsersInRoles_UserProfile` FOREIGN KEY (`UserId`) REFERENCES `UserProfile` (`UserId`) ON DELETE CASCADE ON UPDATE NO ACTION)";
  $sqlQuery=executeQuery($dbh,$query);
}

sub dropLSTables {
  $dbh=$_[0];

  $query="DROP VIEW PipelineInfo, PipelineStatuses";
  $sqlQuery=executeQuery($dbh,$query);
  
  $query="DROP TABLE PipelineMetaData, PipelineServers";
  $sqlQuery=executeQuery($dbh,$query);
  $query="DROP TABLE PipelineDefinitions, Servers, PipelinePending";
  $sqlQuery=executeQuery($dbh,$query);

  $query="DROP TABLE ChannelDefinitions, ChannelPipelines, ChannelMetaData";
  $sqlQuery=executeQuery($dbh,$query);

  $query="DROP TABLE webpages_Membership, webpages_OAuthMembership, webpages_UsersInRoles";
  $sqlQuery=executeQuery($dbh,$query);
  $query="DROP TABLE UserProfile, webpages_OAuthToken, webpages_Roles";
  $sqlQuery=executeQuery($dbh,$query);
}

1;
