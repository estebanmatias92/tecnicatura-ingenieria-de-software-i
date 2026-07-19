/**
*    Project     : Sample Vault
*    Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
*    License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
*    Date        : Marzo 2026
*/

/**
 * Importar módulo/"biblioteca" para gestionar base de datos
 */
const mysql = require('mysql2');

/**
 * Crear un pool de conexiones (más eficiente que una conexión única)
 * se usan variables de entorno  del archivo .env (más seguro).
 */
const pool = mysql.createPool({
    host: process.env.DB_HOST,
    user: process.env.DB_USER,
    password: process.env.DB_PASS,
    database: process.env.DB_NAME,
    waitForConnections: true,
    connectionLimit: 10,
    queueLimit: 0
});

/**
 * Sin usar archivo .env, se expondrían datos sensibles
 */
// const pool = mysql.createPool({
//     host: 'localhost',
//     user: 'samplevault',
//     password: 'samplevault',
//     database: 'samplevault',
//     waitForConnections: true,
//     connectionLimit: 10,
//     queueLimit: 0
// });

// Exportar la promesa del pool para usar async/await en los repositorios
module.exports = pool.promise();
