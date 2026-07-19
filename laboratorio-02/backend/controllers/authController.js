/**
*    Project     : Sample Vault
*    Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
*    License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
*    Date        : Marzo 2026
*/

const bcrypt = require('bcrypt'); // Biblioteca para encriptar contraseñas
const jwt = require('jsonwebtoken'); // Para generar tokens de sesión
const userRepo = require('../repositories/userRepo'); // Importamos el repositorio de usuarios

/**
 * Deserialización moderna: "Del objeto que devuelve este require, buscá la propiedad que se 
 * llame exactamente SECRET_KEY y creá una constante con ese mismo nombre y valor".
 *  */ 
const { SECRET_KEY } = require('../middleware/authMiddleware'); 

class AuthController 
{
    // Registro de usuarios
    async register(req, res) 
    {
        try 
        {
            const { username, password } = req.body;

            // 1. Validación de presencia
            if (!username || !password) {
                return res.status(400).json({ message: "Usuario y contraseña son requeridos." });
            }

            const hashedPassword = await bcrypt.hash(password, 10);            
            
            // 2. Creación mediante el repositorio (que usa el SP sp_create_user)
            const userId = await userRepo.create(username, hashedPassword, 'producer');
            
            res.status(201).json({ 
                message: "Usuario registrado con éxito.", 
                userId 
            });
        }
        catch (error)
        {
            // 3. Manejo de error específico: Usuario Duplicado (Código ER_DUP_ENTRY en MySQL)
            if (error.code === 'ER_DUP_ENTRY') {
                return res.status(409).json({ message: "El nombre de usuario ya existe." });
            }

            res.status(500).json({ 
                message: "Error interno durante el registro.", 
                error: error.message 
            });
        }
    }

    // Inicio de sesión
    async login(req, res) 
    {
        try
        {
            const { username, password } = req.body;

            if (!username || !password) {
                return res.status(400).json({ message: "Credenciales incompletas." });
            }

            // El repositorio ahora devuelve el usuario con su ROL gracias al JOIN en el SP
            const user = await userRepo.findByUsername(username);

            if (!user || !(await bcrypt.compare(password, user.password)))
            {
                return res.status(401).json({ message: "Credenciales inválidas." });
            }

            // 4. Generación del Token (Payload consistente con el Middleware)
            const token = jwt.sign(
                { 
                    id: user.id, 
                    role: user.role // Este string 'admin' o 'producer' viene de la tabla roles
                }, 
                SECRET_KEY, 
                { expiresIn: '2h' }
            );

            res.json({ 
                message: "Login exitoso.", 
                token, 
                role: user.role 
            });
        }
        catch (error)
        {
            res.status(500).json({ 
                message: "Error en el proceso de autenticación.", 
                error: error.message 
            });
        }
    }
}

module.exports = new AuthController();
