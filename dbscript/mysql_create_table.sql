DROP TABLE IF EXISTS user_chat_record;  
CREATE TABLE user_chat_record (  
  Sender   BIGINT NOT NULL,  
  Receiver BIGINT NOT NULL,  
  MessageID INT(11),  
  DateTime BIGINT,  
  Payload  MEDIUMBLOB
);

DROP TABLE IF EXISTS user_offline_record;  
CREATE TABLE user_offline_record (  
  Sender   BIGINT NOT NULL,  
  Receiver BIGINT NOT NULL,  
  GroupID  BIGINT NOT NULL,  
  MessageID INT(11),  
  DateTime BIGINT,  
  Payload  MEDIUMBLOB
);  

DROP TABLE IF EXISTS group_data;  
CREATE TABLE group_data (  
  groupid   BIGINT NOT NULL,  
  users     VARCHAR(4096)
);

