/**
*    Project     : Sample Vault
*    Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
*    License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
*    Date        : Marzo 2026
*/

const express = require('express');
const router = express.Router();
const authController = require('../controllers/authController');

// Ruta para el registro: POST /api/auth/register
router.post('/register', authController.register);

// Ruta para el login: POST /api/auth/login
router.post('/login', authController.login);

module.exports = router;