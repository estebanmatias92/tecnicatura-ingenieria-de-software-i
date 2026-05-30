/**
*    Project     : Sample Vault
*    Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
*    License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
*    Date        : Marzo 2026
*/

const express = require('express');
const router = express.Router();
const adminController = require('../controllers/adminController');

// Importamos los middlewares centralizados
const { verifyToken, isAdmin } = require('../middleware/authMiddleware');

/**
 * Aplicamos la cadena de seguridad a todas las rutas:
 * 1. ¿Quién sos? (verifyToken)
 * 2. ¿Tenés permiso? (isAdmin)
 */
router.use(verifyToken, isAdmin);

router.get('/users', adminController.getAllUsers);
router.delete('/users/:id', adminController.deleteUser);

module.exports = router;