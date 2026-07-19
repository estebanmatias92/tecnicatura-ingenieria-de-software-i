/**
* Project     : Sample Vault
* Author      : Tecnologías Informáticas B - Facultad de Ingeniería - UNMdP
* License     : http://www.gnu.org/licenses/gpl.txt  GNU GPL 3.0
* Date        : Marzo 2026
*/

const db = require('../config/db');

class SampleRepository 
{
    // Insertar un nuevo registro de sample
    async create(sampleData) 
    {
        const { user_id, filename, display_name, category, bpm, file_path } = sampleData;
        const [rows] = await db.execute(
            'CALL sp_create_sample(?, ?, ?, ?, ?, ?)', 
            [user_id, filename, display_name, category, bpm, file_path]
        );
        return rows[0][0].insertId;
    }

    // Obtener todos los samples de un productor específico
    async findByUserId(userId) 
    {
        const [rows] = await db.execute('CALL sp_find_samples_by_user(?)', [userId]);
        return rows[0];
    }

    // Buscar y obtener un sample validando el ID y el propietario
    async findById(id, userId) 
    {
        const [rows] = await db.execute('CALL sp_find_sample_by_id(?, ?)', [id, userId]);
        return rows[0][0]; 
    }

    // Eliminar un sample validando la propiedad del mismo
    async delete(id, userId) 
    {
        await db.execute('CALL sp_delete_sample(?, ?)', [id, userId]);
        return true;
    }
}

module.exports = new SampleRepository();
