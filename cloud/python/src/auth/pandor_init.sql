CREATE USER 'pandor_adm'@'%' IDENTIFIED BY 'Z4TsHC?uE0ce,o!';

CREATE DATABASE IF NOT EXISTS pandor_user;

GRANT ALL PRIVILEGES ON pandor_user.* TO 'pandor_adm'@'%';

USE pandor_user;

CREATE TABLE IF NOT EXISTS user_roles (
    role_id INT PRIMARY KEY AUTO_INCREMENT,
    role_name VARCHAR(50) UNIQUE NOT NULL
);

INSERT INTO user_roles (role_name) VALUES ('customer');
INSERT INTO user_roles (role_name) VALUES ('worker');
INSERT INTO user_roles (role_name) VALUES ('operator');
INSERT INTO user_roles (role_name) VALUES ('admin');

CREATE TABLE IF NOT EXISTS users (
    user_id INT PRIMARY KEY AUTO_INCREMENT,
    username VARCHAR(50) UNIQUE NOT NULL,
    email VARCHAR(100) UNIQUE NOT NULL,
    password VARCHAR(255) NOT NULL,
    salt VARCHAR(255) NOT NULL, -- New field for storing salt
    role_id INT,
    registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    last_login_date DATETIME,
    FOREIGN KEY (role_id) REFERENCES user_roles(role_id)
);