/**
*    Project     : Sample Vault
*    Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
*    License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
*    Date        : Marzo 2026
*/

const express = require('express');
const router = express.Router();
const sampleController = require('../controllers/sampleController');

//configuración de Multer para subir archivos de audio:
const uploadMiddleware = require('../config/multerConfig');

const { verifyToken } = require('../middleware/authMiddleware');

// Todas las rutas de samples requieren que el usuario esté logueado
router.use(verifyToken);

// Subir un nuevo audio: POST /api/samples/upload
// 'audioFile' es el nombre que debe tener el campo file en el FormData del frontend
router.post('/upload', uploadMiddleware, sampleController.uploadSample);

// Listar mis samples: GET /api/samples/my-samples
router.get('/my-samples', sampleController.getMySamples);

// Eliminar un sample: DELETE /api/samples/:id
router.delete('/:id', sampleController.deleteSample);

module.exports = router;