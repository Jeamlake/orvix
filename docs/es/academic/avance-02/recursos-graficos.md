# Recursos gráficos del Avance 2

Método: herramienta integrada de generación de imágenes. No se utilizó la API ni un CLI externo. La portada es una ilustración conceptual y los mockups son propuestas, no capturas de software implementado. Las figuras de flujo, arquitectura, IPC y datos proceden de diagramas editables de la presentación.

## Regeneración de documentos

Los generadores Python requieren `python-docx`; ejecuta `build_document.py` o
`build_implementation_addendum.py` desde un entorno con esa dependencia.
El generador de PowerPoint utiliza `@oai/artifact-tool` del runtime de documentos
y `container_tools/artifact_tool_utils.mjs` de la habilidad de presentaciones.
Configura `ORVIX_ARTIFACT_RUNTIME` con la carpeta de dependencias que contiene
`node/node_modules` y `ORVIX_PRESENTATION_SKILL` con la carpeta de esa habilidad,
y ejecuta `node build_presentation.mjs`. Ningún script requiere la ruta personal
de la computadora donde se preparó el avance.

## Archivos guardados

- C:/Desarrollo/Proyectos/orvix/docs/es/academic/avance-02/assets/portada.png
- C:/Desarrollo/Proyectos/orvix/docs/es/academic/avance-02/assets/mockup-captura.png
- C:/Desarrollo/Proyectos/orvix/docs/es/academic/avance-02/assets/mockup-historial.png

## Prompt de portada

Use case: stylized-concept. Asset type: background illustration for a premium academic 16:9 presentation about ORVIX, a native camera video ingestion application. Generate one wide cinematic abstract product illustration. Very dark midnight charcoal background, a precisely rendered optical camera lens on the right, translucent violet and emerald green ribbons representing video frames flowing through three subtle glass memory layers toward an elegant luminous data stream. Clean sophisticated 3D studio rendering, restrained light, sharp lens detailing, lots of nearly black negative space across left 55 percent for editable slide title. Palette deep violet and mint emerald against #0C1017. No typography, no letters, no logos, no numeric measurements, no UI panels, no people, no watermark. This is conceptual art, not a photograph of a real ORVIX product.

## Prompt de captura

Use case: ui-mockup. Asset type: proposed ORVIX desktop application interface for an academic presentation. Generate one crisp front-on widescreen 16:9 desktop UI mockup, no device frame, no perspective. Tasteful comfortable professional dark interface, charcoal background, subtle violet and emerald accents, white large readable Spanish labels. Left narrow navigation labels 'ORVIX', 'Inicio', 'Captura', 'Historial', 'Ajustes'. Main heading 'Captura en vivo'. Main large video preview contains a clearly synthetic abstract test pattern made of soft gradient rectangles, no person and no fake webcam photograph. Small top configuration strip 'Fuente sintética', '1280 × 720', 'RAW'. Control buttons 'Iniciar captura', 'Detener', 'Reconectar'. Four restrained metric labels 'FPS de captura', 'FPS del visor', 'Frames omitidos', 'Uso del búfer' with em dash placeholders instead of numbers. One short muted status message 'Listo para iniciar'. Flat elegant spacious interface, coherent typographic hierarchy, accurate Spanish spelling, no decorative circuits, no fake telemetry charts, no logos of other brands, no watermark. This is a design proposal, not a screenshot of implemented software. Keep all essential content comfortably inside margins.

## Prompt de historial

Use case: ui-mockup. Asset type: second distinct ORVIX desktop application interface design proposal, session history, for an academic presentation. Generate one crisp front-on widescreen 16:9 desktop UI mockup with no perspective and no device border. Match premium comfortable charcoal dark interface with restrained violet and emerald green accents, highly legible large Spanish labels. Narrow left navigation shows 'ORVIX', 'Inicio', 'Captura', 'Historial', 'Ajustes', with Historial selected. Main heading 'Historial de sesiones'. Empty state for proposed SQLite persistence. Elegant uncluttered area says 'Aún no hay sesiones guardadas' and small explanation 'Las sesiones aparecerán aquí cuando se implemente la telemetría'. A compact top bar has 'Buscar sesión', 'Fuente', 'Modo', 'Exportar CSV'. Underneath, a simple empty readable table with headers 'Sesión', 'Fuente', 'Inicio', 'Duración', 'FPS medio', 'Estado', but no records or fake statistics. Lower right secondary action 'Volver a captura'. No line charts, no invented results, no screenshots of real software, no external brand logos, no watermark, no dense dashboard cards. Spacious polished typographic hierarchy and visually calm UI.
