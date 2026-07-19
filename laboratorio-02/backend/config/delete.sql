-- Detener el uso de la base de datos si está activa
USE master; -- O cualquier base de datos de sistema

-- Eliminar la base de datos completa
DROP DATABASE IF EXISTS samplevaultestrf1;

-- Opcional: Eliminar un usuario específico si se creó uno para este proyecto
DROP USER IF EXISTS 'samplevaultestrf1'@'localhost';

-- Confirmación de limpieza
SELECT 'Database deleted successfully' AS Status;
