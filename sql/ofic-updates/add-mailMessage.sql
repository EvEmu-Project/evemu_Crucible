DROP TABLE IF EXISTS mailMessage;

CREATE TABLE mailMessage
(
  messageID INT NOT NULL AUTO_INCREMENT,
  senderID BIGINT,
  toCharacterIDs TEXT,
  toListID INT,
  toCorpOrAllianceID INT,
  title TEXT,
  body BLOB,
  sentDate BIGINT,
  statusMask TINYINT,
  labelMask INT,
  unread TINYINT,
  PRIMARY KEY (messageID)
);
