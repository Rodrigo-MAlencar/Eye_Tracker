// ============================================================
//  DETECTOR DE ROSTO E OLHOS — Versão Simples para Iniciantes
// ============================================================
//
//  O que este programa faz:
//    1. Abre a câmera do computador
//    2. Detecta o rosto e desenha um RETÂNGULO VERDE em volta
//    3. Dentro do rosto, detecta os olhos e desenha
//       RETÂNGULOS AZUIS em volta de cada olho
//    4. Tudo acompanha o movimento em tempo real
//    5. Pressione ESC para fechar o programa
//
//  Dependência: OpenCV (biblioteca de visão computacional)
//  Não precisa de inteligência artificial externa —
//  o próprio OpenCV já inclui os detectores prontos (.xml)
// ============================================================

// -- Inclui a biblioteca principal do OpenCV -----------------
#include <opencv2/opencv.hpp>

// -- Inclui o módulo de detecção de objetos do OpenCV --------
#include <opencv2/objdetect.hpp>

// -- Inclui recursos básicos do C++ --------------------------
#include <iostream>

// ============================================================
//  CONFIGURAÇÕES — altere aqui se precisar
// ============================================================

// Índice da câmera: 0 = câmera padrão do notebook
// Troque para 1 ou 2 se tiver mais de uma câmera conectada
const int CAMERA_INDEX = 0;

// Caminhos dos arquivos de detecção (XML)
// Esses arquivos DEVEM estar na mesma pasta que o .exe
// Você os encontra dentro da pasta do OpenCV que baixou:
//   opencv\build\etc\haarcascades
const std::string ARQUIVO_ROSTO = "haarcascade_frontalface_default.xml";
const std::string ARQUIVO_OLHOS = "haarcascade_eye.xml";

// Cores dos retângulos (formato BGR — Blue, Green, Red)
const cv::Scalar COR_ROSTO(0, 255, 0);   // Verde
const cv::Scalar COR_OLHOS(255, 0, 0);   // Azul
const int ESPESSURA_LINHA = 2;            // Espessura em pixels


// ============================================================
//  FUNÇÃO PRINCIPAL — o programa começa aqui
// ============================================================
int main()
{
    std::cout << "Iniciando detector de rosto e olhos..." << std::endl;
    std::cout << "Pressione ESC para sair." << std::endl;


    // --------------------------------------------------------
    //  PASSO 1: Carregar os detectores (arquivos XML)
    //
    //  O OpenCV usa um método chamado Haar Cascade para
    //  detectar rostos e olhos. Ele já vem treinado —
    //  basta carregar o arquivo .xml correspondente.
    // --------------------------------------------------------

    cv::CascadeClassifier detectorRosto;
    cv::CascadeClassifier detectorOlhos;

    // Tenta carregar o detector de rosto
    if (!detectorRosto.load(ARQUIVO_ROSTO)) {
        std::cerr << "ERRO: Arquivo nao encontrado: " << ARQUIVO_ROSTO << std::endl;
        std::cerr << "Copie o arquivo .xml para a mesma pasta do programa." << std::endl;
        return 1; // encerra o programa com erro
    }

    // Tenta carregar o detector de olhos
    if (!detectorOlhos.load(ARQUIVO_OLHOS)) {
        std::cerr << "ERRO: Arquivo nao encontrado: " << ARQUIVO_OLHOS << std::endl;
        std::cerr << "Copie o arquivo .xml para a mesma pasta do programa." << std::endl;
        return 1;
    }

    std::cout << "Detectores carregados com sucesso!" << std::endl;


    // --------------------------------------------------------
    //  PASSO 2: Abrir a câmera
    //
    //  VideoCapture é a classe do OpenCV que acessa a câmera.
    //  Passamos o índice 0 para usar a câmera padrão.
    // --------------------------------------------------------

    cv::VideoCapture camera(CAMERA_INDEX);

    if (!camera.isOpened()) {
        std::cerr << "ERRO: Nao foi possivel abrir a camera." << std::endl;
        std::cerr << "Verifique se a camera esta conectada e nao esta em uso." << std::endl;
        return 1;
    }

    std::cout << "Camera aberta com sucesso!" << std::endl;


    // --------------------------------------------------------
    //  PASSO 3: Loop principal — processa frame por frame
    //
    //  A câmera captura ~30 imagens por segundo (frames).
    //  Para cada frame, fazemos:
    //    a) Capturar a imagem da câmera
    //    b) Converter para escala de cinza (detectores exigem)
    //    c) Detectar o rosto
    //    d) Detectar os olhos dentro do rosto
    //    e) Desenhar os retângulos
    //    f) Mostrar na tela
    // --------------------------------------------------------

    // Variáveis que armazenam as imagens
    cv::Mat frameColorido;   // imagem original (colorida) — exibida na tela
    cv::Mat frameCinza;      // imagem em cinza — usada para detectar

    while (true) // loop infinito — só sai com ESC
    {
        // -- a) Captura um frame da câmera -------------------
        camera >> frameColorido;

        // Se o frame estiver vazio, a câmera desconectou
        if (frameColorido.empty()) {
            std::cerr << "AVISO: Frame vazio. Encerrando." << std::endl;
            break;
        }

        // Espelha a imagem horizontalmente
        // (fica mais natural, como olhar num espelho)
        cv::flip(frameColorido, frameColorido, 1);


        // -- b) Converte para escala de cinza ----------------
        //
        //  Os detectores Haar Cascade trabalham apenas com
        //  imagens em preto e branco. A cor não é necessária
        //  para detectar formas e bordas.
        //
        cv::cvtColor(frameColorido, frameCinza, cv::COLOR_BGR2GRAY);

        //  equalizeHist melhora o contraste da imagem em cinza.
        //  Ajuda a detectar o rosto em diferentes iluminações.
        cv::equalizeHist(frameCinza, frameCinza);


        // -- c) Detectar rostos ------------------------------
        //
        //  detectMultiScale percorre a imagem em vários
        //  tamanhos e retorna uma lista de retângulos (Rect)
        //  onde encontrou rostos.
        //
        //  Parâmetros principais:
        //    1.1  = reduz a imagem 10% a cada passo de busca
        //    5    = precisa de 5 confirmações para detectar
        //           (mais alto = menos falsos positivos)
        //    (80,80) = tamanho mínimo do rosto em pixels
        //
        std::vector<cv::Rect> rostos;
        detectorRosto.detectMultiScale(
            frameCinza,      // imagem de entrada
            rostos,          // lista de rostos encontrados
            1.1,             // fator de escala
            5,               // vizinhos mínimos
            0,               // flags (deixe 0)
            cv::Size(80, 80) // tamanho mínimo
        );


        // -- d) Para cada rosto encontrado -------------------
        for (const cv::Rect& rosto : rostos)
        {
            // Desenha o retângulo VERDE em volta do rosto
            cv::rectangle(frameColorido, rosto, COR_ROSTO, ESPESSURA_LINHA);

            // Recorta só a região do rosto na imagem cinza
            // Isso é a "ROI" — Region of Interest (Região de Interesse)
            // Buscar olhos só dentro do rosto é muito mais rápido
            // e evita detectar falsamente olhos em outras partes
            cv::Mat regiaoRosto = frameCinza(rosto);

            // -- e) Detectar olhos dentro do rosto -----------
            std::vector<cv::Rect> olhos;
            detectorOlhos.detectMultiScale(
                regiaoRosto,   // imagem da região do rosto
                olhos,         // lista de olhos encontrados
                1.1,           // fator de escala
                4,             // vizinhos mínimos
                0,             // flags
                cv::Size(20, 20) // tamanho mínimo do olho
            );

            // Limita a no máximo 2 olhos por rosto
            // (evita detectar sobrancelhas como olhos)
            int contadorOlhos = 0;

            for (const cv::Rect& olho : olhos)
            {
                if (contadorOlhos >= 2) break;

                // O retângulo do olho está em coordenadas
                // RELATIVAS ao rosto. Precisamos converter para
                // coordenadas do frame completo somando o offset
                // do rosto (rosto.x e rosto.y).
                cv::Rect olhoNoFrame(
                    rosto.x + olho.x,  // posição X no frame completo
                    rosto.y + olho.y,  // posição Y no frame completo
                    olho.width,        // largura do retângulo
                    olho.height        // altura do retângulo
                );

                // Desenha o retângulo AZUL em volta do olho
                cv::rectangle(frameColorido, olhoNoFrame, COR_OLHOS, ESPESSURA_LINHA);

                contadorOlhos++;
            }
        }


        // -- f) Mostra instruções na tela --------------------
        cv::putText(
            frameColorido,
            "Pressione ESC para sair",
            cv::Point(10, 25),           // posição do texto
            cv::FONT_HERSHEY_SIMPLEX,    // fonte
            0.6,                         // tamanho
            cv::Scalar(200, 200, 200),   // cor (cinza claro)
            1                            // espessura
        );

        // Mostra quantos rostos foram detectados
        std::string info = "Rostos: " + std::to_string(rostos.size());
        cv::putText(frameColorido, info, cv::Point(10, 50),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 1);


        // -- g) Exibe o frame na janela ----------------------
        cv::imshow("Detector de Rosto e Olhos", frameColorido);


        // -- h) Verifica se o usuário pressionou ESC ---------
        //  waitKey(1) espera 1ms e retorna o código da tecla.
        //  O código do ESC é 27.
        if (cv::waitKey(1) == 27) {
            std::cout << "ESC pressionado. Encerrando..." << std::endl;
            break;
        }

    } // fim do loop principal


    // --------------------------------------------------------
    //  PASSO 4: Liberar recursos
    //
    //  Boa prática: sempre libere a câmera e feche as janelas
    //  ao terminar o programa.
    // --------------------------------------------------------
    camera.release();
    cv::destroyAllWindows();

    std::cout << "Programa encerrado." << std::endl;
    return 0;
}