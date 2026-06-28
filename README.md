# Detector de Rosto e Olhos em Tempo Real

Programa em C++ que abre a câmera do computador e detecta rostos e olhos em tempo real:

- 🟩 **Retângulo verde** — contorna o rosto detectado
- 🟦 **Retângulos azuis** — contornam cada olho dentro do rosto

Desenvolvido com **OpenCV** e **Haar Cascade Classifiers**. Não requer internet nem serviços externos — tudo roda localmente.

---

## Como rodar sem instalar nada

Se você **só quer executar** o programa sem editar o código:

1. Clone ou baixe o repositório
2. Abra a pasta `eyetracker`
3. Dê duplo clique em `run.bat`

Todos os arquivos necessários já estão incluídos no repositório.

> Pressione **ESC** para fechar o programa.

---

## Como editar e compilar o código

Siga este guia completo se quiser modificar o código-fonte e recompilar.

---

### Pré-requisitos

Você vai precisar instalar três coisas: **MSYS2**, **OpenCV** e **VS Code**.

---

### Passo 1 — Instalar o MSYS2

O MSYS2 é o ambiente que fornece o compilador C++ e o OpenCV para Windows.

1. Acesse `https://www.msys2.org` e baixe o instalador `.exe`
2. Execute o instalador e siga os passos — mantenha o caminho padrão `C:\msys64`
3. Ao terminar, o MSYS2 abre automaticamente um terminal

---

### Passo 2 — Instalar o OpenCV e o GDB pelo MSYS2

Com o terminal **MSYS2 MinGW x64** aberto, rode este comando:

```bash
pacman -S mingw-w64-x86_64-opencv mingw-w64-x86_64-qt6-base mingw-w64-x86_64-gdb
```

Quando aparecer a pergunta `Proceed with installation?`, digite `S` e pressione Enter.

Aguarde o download e a instalação terminar — pode demorar alguns minutos dependendo da internet.

> **O que esse comando instala:**
> - `opencv` — biblioteca de visão computacional usada no código
> - `qt6-base` — necessário para o OpenCV exibir janelas no Windows
> - `gdb` — debugger para usar o F5 no VS Code

---

### Passo 3 — Adicionar o MSYS2 ao PATH do Windows

Este passo permite que o Windows encontre o compilador e as DLLs automaticamente.

1. Pressione **Win + R**, cole `sysdm.cpl` e pressione Enter
2. Clique na aba **Avançado**
3. Clique em **Variáveis de Ambiente**
4. Na seção **Variáveis do sistema** (parte de baixo), clique em **Path** e depois em **Editar**
5. Clique em **Novo** e cole o caminho abaixo:

```
C:\msys64\mingw64\bin
```

6. Clique **OK** em todas as janelas abertas
7. **Reinicie o computador** — o PATH novo só é reconhecido após reiniciar

Para confirmar que funcionou, abra o **Prompt de Comando** após reiniciar e rode:

```cmd
g++ --version
```

A saída deve mostrar algo como `g++ (Rev..., Built by MSYS2 project) 15.x.x`.

---

### Passo 4 — Instalar o VS Code

1. Acesse `https://code.visualstudio.com` e baixe o instalador para Windows
2. Execute o instalador com as opções padrão
3. Abra o VS Code
4. Pressione **Ctrl+Shift+X** para abrir a aba de extensões
5. Pesquise por `C/C++` e instale a extensão da **Microsoft** (identificador: `ms-vscode.cpptools`)

---

### Passo 5 — Clonar o repositório

Abra o **Prompt de Comando** e rode:

```cmd
git clone <link-do-repositório>
```

Depois abra a pasta no VS Code:

```cmd
cd eyetracker
code .
```

---

### Passo 6 — Ajustar o caminho do compilador

Este é o único passo que muda de máquina para máquina. O caminho do `g++` pode ser diferente no seu computador.

Primeiro, descubra onde o `g++` está instalado:

```cmd
where g++
```

Anote o caminho que aparecer. Agora abra os 3 arquivos dentro da pasta `.vscode` e substitua o caminho pelo que apareceu no comando acima.

> Se o `where g++` retornou `C:\msys64\mingw64\bin\g++.exe`, os arquivos abaixo já estão corretos e você não precisa alterar nada.

---

**`.vscode/c_cpp_properties.json`** — ajuste a linha `compilerPath`:

```json
"compilerPath": "C:/msys64/mingw64/bin/g++.exe"
```

---

**`.vscode/tasks.json`** — ajuste a linha `command` e o `PATH` dentro de `env`:

```json
"command": "C:/msys64/mingw64/bin/g++.exe",
...
"PATH": "C:/msys64/mingw64/bin;${env:PATH}"
```

---

**`.vscode/launch.json`** — ajuste o `miDebuggerPath` e o `value` dentro de `environment`:

```json
"miDebuggerPath": "C:/msys64/mingw64/bin/gdb.exe",
...
"value": "C:/msys64/mingw64/bin;${env:PATH}"
```

---

### Passo 7 — Compilar e rodar

Com tudo configurado, use os atalhos abaixo dentro do VS Code:

| Atalho | O que faz |
|---|---|
| `Ctrl+Shift+B` | Compila o código e gera o `.exe` |
| `F5` | Compila e executa o programa |
| `ESC` | Fecha a janela da câmera |

A primeira vez use `Ctrl+Shift+B` para compilar. Se aparecer a mensagem `Build finished successfully` no terminal, o código compilou. Depois pressione `F5` para rodar.

---

### Resumo do fluxo completo

```
Instalar MSYS2
      ↓
Abrir MSYS2 MinGW x64 e rodar o pacman
      ↓
Adicionar C:\msys64\mingw64\bin ao PATH do Windows
      ↓
Reiniciar o computador
      ↓
Instalar o VS Code + extensão C/C++
      ↓
git clone do repositório
      ↓
Abrir a pasta no VS Code (code .)
      ↓
Ajustar o caminho do g++.exe nos 3 arquivos .vscode
      ↓
Ctrl+Shift+B para compilar
      ↓
F5 para rodar ✓
```

---

### Problemas comuns

**Erro de DLL ao executar (`0xc0000135`)**
O Windows não encontrou as DLLs do OpenCV. Verifique se o caminho `C:\msys64\mingw64\bin` foi adicionado ao PATH e se o computador foi reiniciado após isso.

**Erro `g++: command not found`**
O compilador não está no PATH. Verifique se o Passo 3 foi feito corretamente e reinicie o computador.

**Câmera não abre**
Verifique se a câmera não está sendo usada por outro programa (Teams, Zoom, etc.). Se tiver mais de uma câmera, troque o valor de `CAMERA_INDEX` no topo do arquivo `.cpp` de `0` para `1`.

**Rosto não é detectado**
Melhore a iluminação do ambiente — luz frontal difusa funciona melhor. Evite fundos muito iluminados atrás da câmera.

**Muitos falsos positivos (detecta objetos que não são rostos)**
Aumente o valor do parâmetro `minNeighbors` no `detectMultiScale` do rosto de `5` para `7` ou `8` no arquivo `.cpp`.

---

## Estrutura do repositório

```
eyetracker/
├── .vscode/
│   ├── c_cpp_properties.json   ← configuração do IntelliSense
│   ├── tasks.json              ← comando de compilação
│   └── launch.json             ← configuração do debug
├── platforms/
│   └── qwindows.dll            ← plugin Qt6 para exibir janelas
├── detector_facial.cpp         ← código-fonte principal
├── detector_facial.exe         ← executável compilado
├── haarcascade_frontalface_default.xml  ← detector de rostos
├── haarcascade_eye.xml         ← detector de olhos
├── libopencv_*.dll             ← DLLs do OpenCV
├── libgcc_s_seh-1.dll          ← DLL do compilador
├── libstdc++-6.dll             ← DLL do compilador
├── libwinpthread-1.dll         ← DLL de threads
├── Qt6Core.dll                 ← DLL do Qt6
├── Qt6Gui.dll                  ← DLL do Qt6
├── Qt6Widgets.dll              ← DLL do Qt6
├── .gitignore
└── README.md
```

---

## Tecnologias utilizadas

- **C++17** — linguagem de programação
- **OpenCV 4.x** — biblioteca de visão computacional
- **Haar Cascade Classifiers** — algoritmo de detecção de objetos em imagens
- **MSYS2 / MinGW-w64** — ambiente de compilação para Windows
- **Qt6** — framework usado pelo OpenCV para exibir janelas no Windows
