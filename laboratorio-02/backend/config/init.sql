SET NAMES utf8;
SET time_zone = '+00:00';
SET foreign_key_checks = 0;
SET sql_mode = 'NO_AUTO_VALUE_ON_ZERO';

-- 1. Borrar la base de datos si existe
DROP DATABASE IF EXISTS samplevaultest;
CREATE DATABASE samplevaultest;
USE samplevaultest;

-- 2. Configuración de usuario de DB (Restricción de privilegios)
-- Primero creamos la identidad (si no existe)
CREATE USER IF NOT EXISTS 'samplevaultest'@'localhost' IDENTIFIED BY 'samplevaultest';

-- Luego asignamos los permisos específicos
-- Arquitectura de seguridad conocida como el "Principio de Menor Privilegio"
-- Aplicamos SELECT para lectura y EXECUTE para poder invocar los Stored Procedures.
-- Esto impide INSERT, UPDATE y DELETE directos desde el código de la aplicación.
GRANT SELECT, EXECUTE ON samplevaultest.* TO 'samplevaultest'@'localhost';

-- 3. Tabla de Roles (Normalización)
CREATE TABLE roles (
    id INT AUTO_INCREMENT PRIMARY KEY,
    name VARCHAR(20) NOT NULL UNIQUE
);

-- 4. Tabla de Usuarios (Sin el campo role ENUM)
CREATE TABLE users (
    id INT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(50) NOT NULL UNIQUE,
    password VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

-- 5. Tabla Intermedia: users_roles (Relación Muchos a Muchos)
CREATE TABLE users_roles (
    user_id INT NOT NULL,
    role_id INT NOT NULL,
    PRIMARY KEY (user_id, role_id),
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE,
    FOREIGN KEY (role_id) REFERENCES roles(id) ON DELETE CASCADE
);

-- 6. Tabla de Samples
CREATE TABLE samples (
    id INT AUTO_INCREMENT PRIMARY KEY,
    user_id INT NOT NULL,
    filename VARCHAR(255) NOT NULL,
    display_name VARCHAR(100) NOT NULL,
    category VARCHAR(50),
    bpm INT DEFAULT 0,
    file_path VARCHAR(255) NOT NULL,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (user_id) REFERENCES users(id) ON DELETE CASCADE
);

-- 7. Inserción de Datos Maestros (Roles)
INSERT INTO roles (name) VALUES ('admin'), ('producer');

-- 8. Datos de prueba iniciales
-- Usuario 'admin' (pass: 12345)
INSERT INTO users (id, username, password) VALUES (1, 'admin', '$2b$10$.n0s847tiSxBqDvIo6Vg5ujXC5zIUmm98bTjBWnRdqX9CxxbIo7wS');
INSERT INTO users_roles (user_id, role_id) VALUES (1, 1); -- Rol Admin

-- Usuario 'pepe' (pass: 12345)
INSERT INTO users (id, username, password) VALUES (2, 'pepe', '$2b$10$.n0s847tiSxBqDvIo6Vg5ujXC5zIUmm98bTjBWnRdqX9CxxbIo7wS');
INSERT INTO users_roles (user_id, role_id) VALUES (2, 2); -- Rol Producer

-- ==========================================================
-- 9. PROCEDIMIENTOS ALMACENADOS (Stored Procedures)
-- ==========================================================

DELIMITER //

-- --- PROCEDIMIENTOS PARA USERS ---

-- Buscar usuario por username (con su rol)
CREATE PROCEDURE sp_find_user_by_username(IN p_username VARCHAR(50))
BEGIN
    SELECT u.*, r.name as role 
    FROM users u
    JOIN users_roles ur ON u.id = ur.user_id
    JOIN roles r ON ur.role_id = r.id
    WHERE u.username = p_username;
END //

-- Crear nuevo usuario y asignar rol por nombre
CREATE PROCEDURE sp_create_user(
    IN p_username VARCHAR(50), 
    IN p_password VARCHAR(255), 
    IN p_role_name VARCHAR(20)
)
BEGIN
    DECLARE v_user_id INT;
    DECLARE v_role_id INT;

    INSERT INTO users (username, password) VALUES (p_username, p_password);
    SET v_user_id = LAST_INSERT_ID();

    SELECT id INTO v_role_id FROM roles WHERE name = p_role_name;
    INSERT INTO users_roles (user_id, role_id) VALUES (v_user_id, v_role_id);
    
    SELECT v_user_id as insertId;
END //

-- Listar todos los usuarios
CREATE PROCEDURE sp_find_all_users()
BEGIN
    SELECT u.id, u.username, r.name as role, u.created_at 
    FROM users u
    JOIN users_roles ur ON u.id = ur.user_id
    JOIN roles r ON ur.role_id = r.id;
END //

-- Borrar usuario
CREATE PROCEDURE sp_delete_user(IN p_id INT)
BEGIN
    DELETE FROM users WHERE id = p_id;
END //

-- --- PROCEDIMIENTOS PARA SAMPLES ---

-- Crear Sample
CREATE PROCEDURE sp_create_sample(
    IN p_user_id INT,
    IN p_filename VARCHAR(255),
    IN p_display_name VARCHAR(100),
    IN p_category VARCHAR(50),
    IN p_bpm INT,
    IN p_file_path VARCHAR(255)
)
BEGIN
    INSERT INTO samples (user_id, filename, display_name, category, bpm, file_path)
    VALUES (p_user_id, p_filename, p_display_name, p_category, p_bpm, p_file_path);
    SELECT LAST_INSERT_ID() as insertId;
END //

-- Listar samples por usuario
CREATE PROCEDURE sp_find_samples_by_user(IN p_user_id INT)
BEGIN
    SELECT * FROM samples WHERE user_id = p_user_id;
END //

-- Buscar sample específico (Validando dueño)
CREATE PROCEDURE sp_find_sample_by_id(IN p_id INT, IN p_user_id INT)
BEGIN
    SELECT * FROM samples WHERE id = p_id AND user_id = p_user_id;
END //

-- Borrar sample (Validando dueño)
CREATE PROCEDURE sp_delete_sample(IN p_id INT, IN p_user_id INT)
BEGIN
    DELETE FROM samples WHERE id = p_id AND user_id = p_user_id;
END //

DELIMITER ;
SET foreign_key_checks = 1;