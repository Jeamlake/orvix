# ORVIX — Avance 2

1. Ejecuta **ORVIX-Avance2-Setup-0.2.1.exe** y sigue el instalador en español.
   No necesitas instalar Python ni compilar el motor.
2. Abre **ORVIX · Avance 2** desde Inicio o el acceso directo que elijas crear.
3. Para demostrar el sistema sin cámara, deja **Demo sintética** y pulsa Iniciar.
4. Para la webcam, selecciónala en Fuente y pulsa Iniciar. Buscar actualiza
   los dispositivos sin abrirlos.
5. Detener libera la cámara. Puedes iniciar de nuevo sin cerrar la aplicación.

La imagen mantiene su proporción. La rueda o los botones +/− amplían;
arrastra para desplazarte. Ajustar, doble clic o Ctrl+0 restablecen la vista.
Ctrl+Enter inicia, Esc detiene, F11 alterna pantalla completa.

Diagnóstico muestra eventos de esta ejecución y permite guardar un archivo
de texto en una ubicación elegida. Los frames no se graban ni se suben.

Este avance incluye Capture + Bridge + RAW. Filtros, detección e histórico
SQLite quedan para avances posteriores, aunque figuren como diseños en la
documentación. Los FPS son medidos: la demo puede dar 30; la webcam validada
dio aproximadamente 19.46 FPS a 1280×720.

Instalación: `%LOCALAPPDATA%/Programs/ORVIX`.
Logs: `%LOCALAPPDATA%/ORVIX/ORVIX/logs`.
Para desinstalar, usa Aplicaciones instaladas de Windows. Se conservan los
logs y preferencias locales; el instalador no añade drivers.

El paquete requiere Windows 10/11 x64 y no tiene firma digital. No desactives
tu antivirus ni cambies permisos para evitar una advertencia.

Código y evidencia: https://github.com/Jeamlake/orvix;
`docs/es/evidence/delivery-02/desktop-validation.md`. El instalador se genera
con `scripts/package-desktop.ps1`; los ejecutables están excluidos de Git.

La carpeta contiene también un ZIP del código fuente del avance, sin entorno
virtual, logs, credenciales ni archivos personales. El anexo de implementación
en Word complementa la documentación y los mockups anteriores.
