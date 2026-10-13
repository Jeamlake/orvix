import fs from 'node:fs/promises';
import path from 'node:path';
import { createRequire } from 'node:module';
import { fileURLToPath, pathToFileURL } from 'node:url';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../../../..');
const source = path.join(root, 'docs/es/academic/avance-02');
const build = path.join(root, 'out/academic-avance-02/build');
const runtime = process.env.ORVIX_ARTIFACT_RUNTIME;
const skill = process.env.ORVIX_PRESENTATION_SKILL;
if (!runtime || !skill) {
  throw new Error('Define ORVIX_ARTIFACT_RUNTIME y ORVIX_PRESENTATION_SKILL para regenerar la presentación. Consulta recursos-graficos.md.');
}
const requireRuntime = createRequire(path.join(path.resolve(runtime), 'node', 'loader.cjs'));
const { Presentation, PresentationFile } = await import(pathToFileURL(requireRuntime.resolve('@oai/artifact-tool')).href);
const { finalizePresentation } = await import(pathToFileURL(path.join(skill, 'container_tools/artifact_tool_utils.mjs')).href);
const assets = path.join(source, 'assets');
const qa = path.join(root, 'out/academic-avance-02/pptx-qa');
const final = path.join(root, 'entregables/Avance2_ORVIX_2026-10-11/Exposicion_ORVIX_Avance2.pptx');
const candidate = path.join(build, 'candidate.pptx');
const FONT = 'Calibri';
const C = { bg:'#0D1118', ink:'#F5F7FB', muted:'#B6C0D0', green:'#69E5BA', purple:'#B397F6', stroke:'#334454', panel:'#151E29', red:'#FFA6A6' };
const commit='ecd60a1937b8a3719ba4a13fb8e2a9bf9a9eaddc';
const repo='https://github.com/Jeamlake/orvix/blob/'+commit+'/';
await fs.mkdir(qa,{recursive:true});

function text(s, value, x,y,w,h,size=28,color=C.ink,bold=false,align='left') {
  const sh=s.shapes.add({geometry:'textbox',name:value.slice(0,55),position:{left:x,top:y,width:w,height:h},fill:'none',line:{fill:'none',width:0}});
  sh.text=value;
  sh.text.style={typeface:FONT,fontSize:size,color,bold,alignment:align,verticalAlignment:'top',autoFit:'none',insets:0,wrap:'square'};
  return sh;
}
function heading(s,title,index,tag='') {
  text(s,title,64,44,1152,108,44,C.ink,true);
  if(tag) text(s,tag,64,654,1070,28,20,C.muted);
  text(s,String(index).padStart(2,'0'),1168,654,48,28,20,C.muted,false,'right');
}
function slide(p,title,tag='') {
  const s=p.slides.add();s.background.fill=C.bg;heading(s,title,p.slides.items.length,tag);return s;
}
function box(s,label,x,y,w,h,{light=false,pending=false,size=28}={}) {
  const sh=s.shapes.add({geometry:'roundRect',name:label,position:{left:x,top:y,width:w,height:h},fill:light?'#F3F6FA':C.panel,line:{fill:pending?(light?'#67509F':C.purple):(light?'#2C8668':C.green),width:2,style:pending?'dashed':'solid'},borderRadius:10});
  sh.text=label;sh.text.style={typeface:FONT,fontSize:size,color:light?'#152C3F':C.ink,bold:true,alignment:'center',verticalAlignment:'middle',insets:12,autoFit:'none'};return sh;
}
function connect(s,a,b,fromSide='right',toSide='left',light=false,pending=false) {
  return s.shapes.connect(a,b,{kind:'straight',fromSide,toSide,line:{fill:light?'#63758A':C.muted,width:2,style:pending?'dashed':'solid'},tail:{type:'triangle',width:'sm',length:'sm'}});
}
async function image(s,name,x,y,w,h,fit='contain') {
 const bytes=await fs.readFile(path.join(assets,name));
 s.images.add({name,blob:bytes.buffer.slice(bytes.byteOffset,bytes.byteOffset+bytes.byteLength),contentType:'image/png',position:{left:x,top:y,width:w,height:h},fit,alt:name.startsWith('mockup')?'Diseño propuesto de la interfaz ORVIX':'Ilustración conceptual de ingesta de video'});
}
function notes(s,body,sources=[]) {s.speakerNotes.text=body+'\n\nFuentes\n'+sources.join('\n');}
function table(s,values,x,y,width,height,colWidths) {
 const t=s.tables.add({rows:values.length,columns:values[0].length,left:x,top:y,width,height,values,columnWidths:colWidths});
 t.borders.assign({fill:C.stroke,width:1,style:'solid'});
 for(let r=0;r<values.length;r++) for(let c=0;c<values[0].length;c++) {
  const cell=t.getCell(r,c);cell.fill=r===0?'#28364A':(r%2?'#151D28':'#1A2532');
  cell.text.style={typeface:FONT,fontSize:24,color:r===0?C.green:C.ink,bold:r===0,verticalAlignment:'middle',insets:12,autoFit:'none'};
 }
 return t;
}
function flow(s,light=false,offset=0) {
 const ys=light?60:218;
 const labels=['Elegir\nfuente','Validar y\nabrir C++','Publicar\nBridge','Leer\nPython','Mostrar\nRAW'];
 const nodes=labels.map((label,i)=>box(s,label,48+i*238,ys+offset,205,102,{light,size:29}));
 for(let i=1;i<nodes.length;i++) connect(s,nodes[i-1],nodes[i],'right','left',light);
 const fault=box(s,'Interrupción\ndel productor',48,ys+190+offset,320,100,{light,pending:true});
 const detect=box(s,'Detectar caída\no ausencia de frames',470,ys+190+offset,330,100,{light,pending:true});
 const recover=box(s,'Informar estado\ny reconectar',900,ys+190+offset,330,100,{light,pending:true});
 connect(s,fault,detect,'right','left',light,true);connect(s,detect,recover,'right','left',light,true);
 text(s,'Ruta de recuperación propuesta',48,ys+315+offset,1000,42,25,light?'#152C3F':C.purple);
}
function architecture(s,light=false) {
 const ys=light?55:220;
 const labels=['Capture\nC++20','Bridge\nMemoria compartida','Vision\nNumPy y OpenCV','Insight\nMétricas y SQLite'];
 const nodes=labels.map((label,i)=>box(s,label,48+i*304,ys,266,115,{light,pending:i===3,size:29}));
 for(let i=1;i<nodes.length;i++) connect(s,nodes[i-1],nodes[i],'right','left',light,i===3);
 const color=light?'#152C3F':C.muted;
 text(s,'Media Foundation\nV4L2\nAVFoundation',48,ys+148,267,123,25,color);
 text(s,'ORVX v1\nTres slots\nLectura del frame reciente',353,ys+148,270,123,25,color);
 text(s,'RAW implementado\nFiltros en planificación',656,ys+148,273,100,25,color);
 text(s,'Métricas básicas actuales\nPersistencia propuesta',960,ys+148,276,100,25,color);
 text(s,'Python recibe muestras del Bridge sin abrir la cámara',48,ys+310,1175,55,29,light?'#152C3F':C.green,true);
}
function ipc(s,light=false) {
 const y=light?55:225;
 const left=box(s,'Productor\nC++',48,y+48,205,105,{light});
 const bus=s.shapes.add({geometry:'rect',name:'Región del anillo compartido',position:{left:300,top:y,width:660,height:200},fill:'none',line:{fill:light?'#63758A':C.stroke,width:2}});
 [0,1,2].forEach(i=>box(s,'Slot '+i+'\nMetadatos\nPayload',320+i*205,y+20,180,160,{light,size:27}));
 const right=box(s,'Consumidor\nPython',1015,y+48,225,105,{light});
 connect(s,left,bus,'right','left',light);connect(s,bus,right,'right','left',light);
 text(s,'Lector del frame más reciente',305,y+217,650,40,29,light?'#152C3F':C.green,true,'center');
 text(s,'Secuencia publicada y consumida\nGeneración estable antes y después de copiar',80,y+280,1120,83,29,light?'#152C3F':C.muted,false,'center');
}
function er(s,light=false) {
 const y=light?20:174;
 const nodes=[
  box(s,'source\nPK source_id\nkind, backend, nombre',50,y,320,164,{light,pending:true,size:27}),
  box(s,'session\nPK session_id\nFK source_id',483,y,320,164,{light,pending:true,size:27}),
  box(s,'metric_sample\nPK sample_id\nFK session_id',913,y,320,164,{light,pending:true,size:27}),
  box(s,'diagnostic_event\nPK event_id\nFK session_id',483,y+245,320,160,{light,pending:true,size:27}),
 ];
 connect(s,nodes[0],nodes[1],'right','left',light);connect(s,nodes[1],nodes[2],'right','left',light);
 connect(s,nodes[1],nodes[3],'bottom','top',light);
 const col=light?'#152C3F':C.green;
 text(s,'1     0..N',385,y+25,90,40,24,col,false,'center');
 text(s,'1     0..N',812,y+25,94,40,24,col,false,'center');
 text(s,'1   0..N',825,y+189,220,45,24,col);
 text(s,'Telemetría local sin guardar fotogramas',55,y+425,1168,46,28,light?'#152C3F':C.muted,false,'center');
}

const p=Presentation.create({slideSize:{width:1280,height:720}});
let s=p.slides.add();s.background.fill=C.bg;await image(s,'portada.png',0,0,1280,720,'cover');
text(s,'ORVIX',64,126,670,118,94,C.ink,true);
text(s,'Avance 2',69,252,510,62,46,C.green,true);
text(s,'Captura nativa y diseño\nde una aplicación de visión',69,330,570,104,33,C.ink);
text(s,'Programación de Interfaces y Dispositivos Periféricos',69,535,900,35,24,C.muted);
text(s,'Andre Alexander Alvarado Lira\nJose Eduardo Alvarado Mejia\nJavier Gomez Correa',69,585,1050,83,23,C.ink);
notes(s,'Presentamos el Avance 2 de ORVIX. La captura nativa y el puente ya funcionan. La exposición diferencia esa base del diseño de la nueva interfaz y de la telemetría persistente. Tiempo orientativo: 40 segundos. Ilustración conceptual de portada.',[repo+'README.es.md','Presentación del primer avance incluida en avance 1.zip']);

s=slide(p,'Caso propuesto y problemática');
text(s,'Una práctica necesita explicar el recorrido de la webcam',64,152,1140,62,30,C.muted);
const old=box(s,'Cámara y procesamiento\nen un mismo prototipo',80,265,465,152,{size:32});
const obs=box(s,'Responsabilidades separadas\ny métricas identificables',732,265,465,152,{size:32});connect(s,old,obs);
text(s,'Formato y lectura quedan\nmenos visibles en la aplicación',80,455,470,100,28,C.muted);
text(s,'ORVIX expone captura,\ntransporte y consumo',732,455,474,100,28,C.green);
notes(s,'El problema es de control y diagnóstico. Una biblioteca de alto nivel resulta útil, pero puede ocultar decisiones de formato y lectura que queremos estudiar en el curso. No afirmamos mayor velocidad que otras soluciones porque todavía faltan benchmarks comparables. Tiempo: 50 segundos.',[repo+'README.es.md']);

s=slide(p,'Objetivos y actores');
text(s,'Aplicación local para capturar, visualizar y analizar video',64,151,1130,76,34,C.green,true);
const objs=[['01','C++ conserva la cámara'],['02','Python consume el contrato IPC'],['03','El usuario entiende estado y rendimiento']];
objs.forEach((a,i)=>{text(s,a[0],74,270+i*103,110,65,48,C.purple,true);text(s,a[1],198,275+i*103,980,73,32);});
text(s,'Operador',74,597,300,38,27,C.green,true);text(s,'Equipo de desarrollo',420,597,430,38,27,C.green,true);text(s,'Evaluador docente',893,597,340,38,27,C.green,true);
notes(s,'El operador elige fuente y modo y consulta resultados. El equipo mantiene el producto y las pruebas. El docente evalúa la relación entre requisitos, evidencia y demostración. La webcam y el sistema operativo son participantes técnicos. No hay cuentas de usuario ni servicio en la nube. Tiempo: 50 segundos.',[repo+'docs/es/requirements/software-requirements.md','Documento académico complementario, sección 2']);

s=slide(p,'Alcance del avance','Base implementada y diseño pendiente tienen evidencias distintas');
text(s,'Base funcional',74,184,500,55,37,C.green,true);text(s,'Diseño propuesto',696,184,514,55,37,C.purple,true);
text(s,'Captura nativa\nBridge compartido\nVisor RAW\nMétricas básicas\nFuente sintética',74,268,500,314,33);
text(s,'Interfaz de escritorio\nFiltros y detección\nHistorial SQLite\nRecuperación del consumidor\nExportación de métricas',696,268,514,314,33);
notes(s,'La Entrega 2 técnica no equivale a ORVIX 1.0. RAW y el puente están presentes en el commit revisado. Los mockups y el modelo de datos definen el producto futuro y se exponen como diseño. Tiempo: 45 segundos.',[repo+'README.es.md',repo+'docs/es/evidence/delivery-02/README.md']);

s=slide(p,'Requerimientos funcionales');
table(s,[['Componente','Responsabilidad','Estado'],['Capture','Fuente, formato y muestras','Base'],['Bridge','Contrato IPC y secuencias','Base'],['Vision','RAW y futuros filtros','Base y plan'],['Insight','Métricas e historial','Base y plan']],64,182,1152,365,[260,650,242]);
text(s,'27 requisitos conservan sus identificadores',64,579,1100,45,32,C.green,true);
notes(s,'El Word desarrolla los 27 RF con estado y criterio de aceptación. Hay 18 funcionalidades base y nueve planificadas según la especificación vigente. Esta clasificación no es un porcentaje de avance del producto. La matriz original necesita las filas RF-011, 012, 014, 016, 018 y 022. Tiempo: 55 segundos.',[repo+'docs/es/requirements/software-requirements.md',repo+'docs/es/requirements/traceability.md']);

s=slide(p,'Requerimientos no funcionales');
table(s,[['Condición','Regla de diseño'],['Rendimiento','1280 × 720 y 30 FPS nominales como objetivo'],['Privacidad','Fotogramas efímeros por defecto'],['Reproducción','Pruebas sintéticas y pasos de compilación'],['Robustez','Errores visibles y salida sin espera indefinida'],['Usabilidad','Controles legibles y respuesta al redimensionar']],64,178,1152,422,[310,842]);
notes(s,'Distinguimos objetivo nominal de rendimiento físico. La latencia y la respuesta de interfaz tienen que medirse cuando exista la nueva aplicación. La ruta heredada de Windows 7 no se transfiere automáticamente a Qt 6. Tiempo: 50 segundos.',[repo+'docs/es/requirements/software-requirements.md','https://doc.qt.io/qt-6.8/supported-platforms.html']);

s=slide(p,'Flujo principal y recuperación');flow(s);
notes(s,'La ruta superior representa el flujo base: seleccionar, validar, publicar, leer y mostrar RAW. La ruta inferior añade una recuperación propuesta ante muerte del productor o ausencia de nuevos frames. Debemos supervisar procesos propios sin bloquear la interfaz. Tiempo: 55 segundos.',[repo+'python/src/orvix/ui/viewer.py','Diagrama de flujo del primer avance y sección 4 del documento actual']);

s=slide(p,'Pantalla de captura','Diseño propuesto de la interfaz de escritorio');await image(s,'mockup-captura.png',64,157,1152,475);
notes(s,'El video conserva el espacio principal. Los controles y métricas quedan fuera de la imagen. Los guiones son estados sin datos, no resultados medidos. RAW estará disponible primero y los filtros solo se habilitarán después de implementar sus requisitos. Este mockup no es una captura de la aplicación existente. Tiempo: 50 segundos.');

s=slide(p,'Historial de sesiones','Diseño propuesto para la persistencia SQLite');await image(s,'mockup-historial.png',64,157,1152,475);
notes(s,'El historial tendrá una fila por sesión y acceso a muestras y diagnósticos. Una sesión vacía debe explicarse, sin inventar registros. Exportar CSV es una propuesta de la interfaz. No guardaremos imágenes automáticamente. Tiempo: 45 segundos.');

s=slide(p,'Arquitectura del proyecto');architecture(s);
notes(s,'Capture tiene una API común con backends específicos. Bridge publica el protocolo ORVX v1. Vision presenta RAW y posteriormente aplicará filtros. Insight reúne métricas existentes y propone persistencia SQLite. La línea discontinua distingue extensión planificada. Python no controla la webcam directamente. Tiempo: 60 segundos.',[repo+'docs/es/architecture/overview.md',repo+'docs/es/adr/0007-sqlite-telemetry.md']);

s=slide(p,'Memoria compartida e IPC');ipc(s);
notes(s,'El anillo contiene tres slots por defecto. El productor publica sin esperar al lector. Python copia el frame más reciente y comprueba la generación antes y después de copiar. Por ello no afirmamos que toda la ruta tenga cero copias. El header global ocupa 128 bytes y cada header de slot 64 bytes. Tiempo: 55 segundos.',[repo+'docs/es/protocol/ipc-v1.md',repo+'python/src/orvix/ipc/protocol.py']);

s=slide(p,'Base de datos y relaciones','Modelo propuesto para RF-021 y RF-022');er(s);
notes(s,'Una fuente puede producir cero o muchas sesiones. Cada sesión puede contener cero o muchas muestras y diagnósticos. source_id y session_id son las claves de relación. Cada conexión SQLite debe habilitar foreign_keys. Conservaremos fechas UTC y tiempo relativo para las muestras. El modelo no está integrado al código actual. Tiempo: 60 segundos.',['Documento académico, sección 6','https://www.sqlite.org/foreignkeys.html',repo+'docs/es/adr/0007-sqlite-telemetry.md']);

s=slide(p,'Planning general y sprints','Plan propuesto con duración a ajustar por el equipo');
table(s,[['Sprint','Objetivo','Resultado'],['S1','Cierre académico y estabilidad','Documentos y pruebas de fallos'],['S2','Aplicación de escritorio','Fuente, sesión, video y estados'],['S3','Visión y persistencia','Filtros, detector y SQLite'],['S4','Validación y distribución','Recuperación y empaquetado']],64,184,1152,376,[145,470,537]);
text(s,'Cada historia vincula requisito, aceptación y evidencia',64,588,1130,45,30,C.green,true);
notes(s,'El plan no atribuye sprints históricos ni responsabilidades individuales no confirmadas. El tablero tendrá Pendiente, En curso, En revisión y Hecho. La disponibilidad del equipo determina duración y responsables. La interfaz necesita primero el coordinador de sesión, y el historial depende del repositorio SQLite. Tiempo: 55 segundos.',['Documento académico, sección 7',repo+'README.es.md']);

s=slide(p,'Evidencia de la base técnica','Revisión local en Windows del 11 de octubre de 2026');
[['16/16','Pruebas nativas'],['6/6','Pruebas Python'],['1/1','Integración sintética']].forEach((m,i)=>{text(s,m[0],70+i*398,233,358,112,77,C.green,true,'center');text(s,m[1],70+i*398,358,358,57,30,C.ink,false,'center');});
text(s,'19.46 FPS',74,485,426,75,52,C.purple,true);
text(s,'Medición física documentada en septiembre\nFormato nominal de 1280 × 720 a 30 FPS',530,489,686,104,29,C.muted);
notes(s,'Las suites actuales pasan con los ejecutables disponibles. No repetimos hoy la prueba de webcam ni una compilación limpia multiplataforma. Los 19.46 FPS provienen de la prueba física del 8 de septiembre. Aprobar pruebas existentes no implica satisfacer todo el producto. Tiempo: 50 segundos.',[repo+'docs/es/evidence/delivery-02/README.md','Resultados locales CTest, pytest e integración del 11 de octubre de 2026']);

s=slide(p,'Trabajo necesario para cerrar el avance');
text(s,'Robustez',74,190,514,58,37,C.purple,true);
text(s,'Detectar caída del productor\nAtender teclado sin nuevos frames\nPropagar fallos de PowerShell',74,277,560,207,31);
text(s,'Cierre académico',721,190,500,58,37,C.green,true);
text(s,'Actualizar trazabilidad\nRevisar los siete apartados\nValidar cámara y ensayar demo',721,277,510,207,31);
text(s,'Correcciones técnicas pendientes',74,561,1142,48,30,C.muted);
notes(s,'La prueba de interrupción mantuvo el visor esperando porque el estado compartido quedó en RUNNING. waitKey solo se atiende con un nuevo frame. El script PowerShell puede ocultar un fallo nativo si pytest termina bien. Los documentos describen esos pendientes, no los declaran resueltos. Tiempo: 55 segundos.',[repo+'python/src/orvix/ui/viewer.py',repo+'scripts/test.ps1','Documento académico, sección 7']);

s=p.slides.add();s.background.fill=C.bg;await image(s,'portada.png',0,0,1280,720,'cover');
text(s,'ORVIX',64,128,650,120,90,C.ink,true);
text(s,'Captura nativa validada\nInterfaz y SQLite en diseño',69,286,680,136,44,C.green,true);
text(s,'Siguiente hito\nAplicación de escritorio con control de sesión',69,486,895,104,31,C.ink);
text(s,'Avance 2 Semana 10',69,637,800,37,25,C.muted);
notes(s,'Cerramos con una base funcional y un diseño de producto que distingue qué existe y qué falta. El siguiente hito es coordinar la sesión desde una interfaz y después integrar visión y persistencia. Tiempo: 30 segundos. La duración total orientativa es de 13 a 15 minutos.',[repo+'README.es.md']);

await (await PresentationFile.exportPptx(p)).save(candidate);
console.log('Draft saved:',candidate);
if(!process.argv.includes('--finalize')) {
 for(const [index,sl] of p.slides.items.entries()) {
  const blob=await p.export({slide:sl,format:'png',scale:1});
  await fs.writeFile(path.join(qa,'slide-'+(index+1)+'.png'),new Uint8Array(await blob.arrayBuffer()));
 }
 for(const [name,draw,height] of [['flujo.png',flow,450],['arquitectura.png',architecture,450],['ipc.png',ipc,450],['modelo-datos.png',er,500]]) {
  const figures=Presentation.create({slideSize:{width:1280,height}});
  const fig=figures.slides.add();fig.background.fill='#FFFFFF';draw(fig,true);
  const png=await figures.export({slide:fig,format:'png',scale:1.5});
  await fs.writeFile(path.join(assets,name),new Uint8Array(await png.arrayBuffer()));
 }
 console.log('Rendered slides:',p.slides.items.length);
} else {
 const result=await finalizePresentation({workspaceDir:root,candidatePath:candidate,finalPath:final,pythonExecutable:path.join(runtime,'python/python.exe'),integrityValidatorPath:path.join(skill,'container_tools/inspect_presentation_package_integrity.py'),layoutValidatorPath:path.join(skill,'container_tools/inspect_presentation_layout_geometry.py'),layoutArgs:['--expected-slide-size-emu','12192000,6858000','--validate-heading-fit','--require-native-table-slide','5','--require-native-table-slide','6','--require-native-table-slide','13'],fontPolicy:{basis:'design',families:[FONT]},requiredNativeTableOwnerSlides:[5,6,13],requiredNativeChartOwnerSlides:[],verifyArtifactToolImport:true,receiptPath:path.join(build,'presentation-validation.json')});
 console.log(JSON.stringify(result));
}
