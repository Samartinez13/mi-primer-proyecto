#include <iostream>
#include <pqxx/pqxx> 
#include <iomanip>  
#include <string>

using namespace std;
using namespace pqxx;

void imprimirResultados(const result& R, const string& titulo) {
    if (R.empty()) {
        cout << "\n--- " << titulo << " ---" << endl;
        cout << "No se encontraron resultados que cumplan los criterios." << endl;
        return;
    }

    cout << "\n====================================================================================================" << endl;
    cout << "  CONSULTA: " << titulo << " (" << R.size() << " filas)" << endl;
    cout << "====================================================================================================" << endl;
    
    for (size_t i = 0; i < R.columns(); ++i) {
        cout << left << setw(25) << R.column_name(i); 
    }
    cout << endl;
    cout << "----------------------------------------------------------------------------------------------------" << endl;

    for (const auto& row : R) {
        for (const auto& field : row) {
            cout << left << setw(25) << (field.is_null() ? "NULL" : field.c_str());
        }
        cout << endl;
    }
    cout << "====================================================================================================" << endl;
}


void conectar_a_postgres() {
    
    const char* conn_string = "dbname=proyecto_cpp user=postgres password=sistemas host=127.0.0.1 port=5432";

    try {
        connection C(conn_string);
        cout << " Conexion a la base de datos '" << C.dbname() << "' establecida exitosamente." << endl;
        
        nontransaction N(C); 
        
        result rs1, rs2, rs3, rs4, rs5;

        cout << "\nEjecutando Consulta 1..." << endl;
        rs1 = N.exec("select * from factura");
        imprimirResultados(rs1, "1. FACTURAS");
        

        cout << "\nEjecutando Consulta 2..." << endl;
        string sql2 = 
            "select i.id_instructor, ed.sede from instructor i " 
            "join deportes d on i.id_instructor = d.id_deporte " 
            "join deportes_espacio_deporte ded on d.id_deporte = ded.id_deporte "
            "join espacio_deporte ed on ded.id_sede_horario = ed.id_sede_horario "
            "order by i.id_instructor asc";
        rs2 = N.exec(sql2);
        imprimirResultados(rs2, "2. INSTRUCTORES Y SEDES");

        
        cout << "\nEjecutando Consulta 3..." << endl;
        string sql3 = 
            "select r.area, u.nombre || ' ' || u.apellidos as nombre_completo, r.fecha_inscripcion "
            "from usuario u join registro r on u.id_usuario=r.id_usuario "
            "order by r.fecha_inscripcion asc";
        rs3 = N.exec(sql3);
        imprimirResultados(rs3, "3. DEPORTE Y USUARIO");


        cout << "\nEjecutando Consulta 4..." << endl;
        string sql4 = 
            "select u.nombre || ' ' || u.apellidos as nombre_completo, f.id_factura, CURRENT_DATE - f.fecha_pago as dias_retraso "
            "from usuario u join factura f on u.id_usuario = f.id_usuario "
            "join estado_factura ef on f.id_estado_factura = ef.id_estado_factura "
            "where ef.estado = 'No pago' " 
            "order by dias_retraso";
        rs4 = N.exec(sql4);
        imprimirResultados(rs4, "4. USUARIO Y DEBER PAGO");
        

        cout << "\nEjecutando Consulta 5 (La mas compleja y funcional)..." << endl;
        string sql5 = 
            "SELECT "
            "    u.sede AS sede_usuario, "
            "    ef.estado AS estado_factura, "
            "    SUM(p.cantidad_elementos_prestamo) AS total_elementos_prestados, "
            "    COUNT(DISTINCT p.id_prestamo) AS total_prestamos "
            "FROM "
            "    usuario u "
            "JOIN "
            "    usuario_prestamo up ON u.id_usuario = up.id_usuario " 
            "JOIN "
            "    prestamo p ON up.id_prestamo = p.id_prestamo " 
            "JOIN "
            "    prestamo_factura pf ON p.id_prestamo = pf.id_prestamo " 
            "JOIN "
            "    factura f ON pf.id_factura = f.id_factura " 
            "JOIN "
            "    estado_factura ef ON f.id_estado_factura = ef.id_estado_factura " 
            "GROUP BY "
            "    u.sede, ef.estado "
            "ORDER BY "
            "    u.sede, total_elementos_prestados DESC;";
            
        rs5 = N.exec(sql5);
        imprimirResultados(rs5, "5. ELEMENTOS PRESTADOS AGRUPADOS POR SEDE Y ESTADO DE PAGO");


        cout << "\nTodas las 5 consultas se realizaron correctamente" << endl;

    } catch (const exception &e) {
        cerr << "\n ERROR CRITICO DE CONEXION O EJECUCION: " << e.what() << endl;
        cerr << "Asegurese de revisar la sintaxis SQL de las consultas restantes." << endl;
    }
}

int main() {
    conectar_a_postgres();
    return 0;
}