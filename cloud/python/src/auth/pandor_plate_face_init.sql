-- Switch to the 'pandor_user' database if not already selected.
USE pandor_user;

-- Create the 'authorized_faces' table if it doesn't already exist.
CREATE TABLE IF NOT EXISTS authorized_faces (
    face_id INT AUTO_INCREMENT PRIMARY KEY,
    user_manager VARCHAR(50) NOT NULL,
    username VARCHAR(50) UNIQUE NOT NULL,
    registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    face_descriptor BLOB
);

-- Create the 'authorized_plates' table if it doesn't already exist.
CREATE TABLE IF NOT EXISTS authorized_plates (
    plate_id INT AUTO_INCREMENT PRIMARY KEY,
    user_manager VARCHAR(50) NOT NULL,
    registration_date DATETIME DEFAULT CURRENT_TIMESTAMP,
    plate_descriptor TEXT
);

FLUSH PRIVILEGES;