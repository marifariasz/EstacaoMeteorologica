#ifndef JAVASCRIPT_H
#define JAVASCRIPT_H

#define JAVA_SCRIPT \
"function atualizarDados() { \n" \
"    fetch('/dados.json') \n" \
"        .then(response => response.json()) \n" \
"        .then(data => { \n" \
"            /* Bloco final de atualização */ \n" \
"            document.querySelector('.temperatura_bmp').textContent = data.temperatura_bmp.toFixed(2) + ' °C'; \n" \
"            document.querySelector('.temperatura_aht').textContent = data.temperatura_aht.toFixed(2) + ' °C'; \n" \
"            document.querySelector('.umidade').textContent = data.umidade.toFixed(2) + ' %'; \n" \
"            document.querySelector('.altitude').textContent = data.altitude.toFixed(1) + ' m'; \n" \
"        }) \n" \
"        .catch(error => console.error('Falha na requisição ou processamento:', error)); \n" \
"} \n" \
"setInterval(atualizarDados, 2000); \n" \
"window.onload = atualizarDados; \n"

#endif /* JAVASCRIPT_H */