#ifndef STYLE_H
#define STYLE_H

#define STYLE_PAGE \
"body { margin: 0; min-height: 100vh; display: flex; flex-direction: column; align-items: center; background: linear-gradient(135deg, #1e3a8a, #3b82f6); font-family: 'Roboto', Arial, sans-serif; color: #ffffff; }" \
"header { text-align: center; padding: 20px 0; }" \
"header h1 { font-size: 2.8rem; font-weight: 700; text-shadow: 0 2px 4px rgba(0,0,0,0.3); margin: 0; }" \
".principal { display: flex; flex-direction: column; align-items: center; width: 95%; max-width: 600px; padding: 30px; background: rgba(255, 255, 255, 0.1); backdrop-filter: blur(10px); border-radius: 15px; box-shadow: 0 4px 20px rgba(0,0,0,0.2); gap: 20px; }" \
".temperatura_bmp, .temperatura_aht, .umidade, .altitude { font-size: 1.4rem; font-weight: 500; padding: 15px; background: rgba(255, 255, 255, 0.15); border-radius: 10px; width: 100%; text-align: center; box-shadow: 0 2px 5px rgba(0,0,0,0.2); }" \
".estado-led { display: flex; justify-content: space-between; align-items: center; width: 100%; padding: 15px; background: rgba(255, 255, 255, 0.15); border-radius: 10px; box-shadow: 0 2px 5px rgba(0,0,0,0.2); }" \
".led-ligado { width: 28px; height: 28px; border-radius: 50%; background: #22c55e; box-shadow: 0 0 15px #22c55e; transition: all 0.3s ease; }" \
".led-desligado { width: 28px; height: 28px; border-radius: 50%; background: #ef4444; box-shadow: 0 0 15px #ef4444; transition: all 0.3s ease; }" \
"#MinMax { width: 100%; padding: 15px; background: rgba(255, 255, 255, 0.15); border-radius: 10px; text-align: center; font-size: 1.3rem; font-weight: 500; box-shadow: 0 2px 5px rgba(0,0,0,0.2); }" \
".Definicao { width: 100%; display: flex; justify-content: center; }" \
"#camposNumeros { display: flex; flex-direction: column; width: 100%; padding: 20px; background: rgba(255, 255, 255, 0.1); border-radius: 10px; gap: 15px; }" \
"#camposNumeros label { font-weight: 500; color: #ffffff; margin-bottom: 8px; }" \
"#camposNumeros input { padding: 12px; border: none; border-radius: 6px; font-size: 1.1rem; background: rgba(255, 255, 255, 0.9); color: #1e3a8a; outline: none; width: 100%; box-sizing: border-box; }" \
"#camposNumeros input:focus { box-shadow: 0 0 8px rgba(59, 130, 246, 0.5); }" \
"button { padding: 12px 30px; border: none; border-radius: 6px; background: #3b82f6; color: #ffffff; font-size: 1.1rem; font-weight: 500; cursor: pointer; transition: background 0.3s ease, transform 0.2s ease; }" \
"button:hover { background: #2563eb; transform: translateY(-2px); }" \
"button:active { transform: translateY(0); }" \
"input[type=\"number\"]::-webkit-outer-spin-button, input[type=\"number\"]::-webkit-inner-spin-button { -webkit-appearance: none; margin: 0; }" \
"input[type=\"number\"] { -moz-appearance: textfield; }" \
"@media (max-width: 600px) { .principal { width: 98%; padding: 20px; } header h1 { font-size: 2.2rem; } .temperatura_bmp, .temperatura_aht, .umidade, .altitude { font-size: 1.2rem; padding: 12px; } #camposNumeros input { font-size: 1rem; } button { font-size: 1rem; padding: 10px 20px; } }"

#endif