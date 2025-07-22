#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

#define JAVA_SCRIPT \
"function atualizarDados() { \n" \
"    fetch('/dados') \n" \
"        .then(response => response.json()) \n" \
"        .then(data => { \n" \
"            /* Bloco de atualização corrigido */ \n" \
"            document.querySelector('#temp_bmp').textContent = data.temperatura_bmp.toFixed(2) + ' °C'; \n" \
"            document.querySelector('#temp_aht').textContent = data.temperatura_aht.toFixed(2) + ' °C'; \n" \
"            document.querySelector('#humidity').textContent = data.umidade.toFixed(2) + ' %'; \n" \
"            document.querySelector('#altitude').textContent = data.altitude.toFixed(1) + ' m'; \n" \
"            /* Adicione aqui os outros campos se necessário, como 'pump', 'orange_min', etc. */ \n" \
"            document.querySelector('#pump').textContent = data.bomba ? 'Ligada' : 'Desligada'; \n" \
"            document.querySelector('#orange_min').textContent = data.orange_min.toFixed(1); \n" \
"            document.querySelector('#orange_max').textContent = data.orange_max.toFixed(1); \n" \
"        }) \n" \
"        .catch(error => console.error('Falha na requisição ou processamento:', error)); \n" \
"} \n" \
"setInterval(atualizarDados, 2000); \n" \
"window.onload = atualizarDados; \n"

#endif /* JAVASCRIPT_H */