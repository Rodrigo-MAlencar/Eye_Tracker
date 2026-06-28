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
// ============================================================

// -- Windows API: necessária para forçar o carregamento
//    correto das DLLs da pasta do programa ------------------
#include <windows.h>
#include <shlwapi.h>

// -- Biblioteca principal do OpenCV -------------------------
#include <opencv2/opencv.hpp>

// -- Módulo de detecção de objetos do OpenCV ----------------
#include <opencv2/objdetect.hpp>

// -- Recursos básicos do C++ --------------------------------
#include <iostream>

// ============================================================
//  CONFIGURAÇÕES — altere aqui se precisar
// ============================================================

// Índice da câmera: 0 = câmera padrão do notebook
// Troque para 1 ou 2 se tiver mais de uma câmera conectada
const int CAMERA_INDEX = 0;

// Caminhos dos arquivos de detecção (XML)
// Devem estar na mesma pasta que o .exe
const std::string ARQUIVO_ROSTO = "haarcascade_frontalface_default.xml";
const std::string ARQUIVO_OLHOS = "haarcascade_eye.xml";

// Cores dos retângulos (formato BGR: Blue, Green, Red)
const cv::Scalar COR_ROSTO(0, 255, 0);   // Verde
const cv::Scalar COR_OLHOS(255, 0, 0);   // Azul
const int ESPESSURA_LINHA = 2;


// ============================================================
//  FUNÇÃO PRINCIPAL
// ============================================================
int main()
{
    // --------------------------------------------------------
    //  CORREÇÃO DE DLL — deve ser a primeira coisa no main()
    //
    //  Problema: quando o colaborador tem o MSYS2 instalado
    //  na máquina dele, o Windows pode carregar as DLLs da
    //  instalação DELE em vez das DLLs da pasta do projeto.
    //  Se as versões forem diferentes, o programa quebra.
    //
    //  Solução: SetDllDirectoryW() diz ao Windows para sempre
    //  buscar DLLs na pasta do .exe PRIMEIRO, antes de
    //  qualquer outra pasta do sistema ou do PATH.
    //
    //  Resultado: o programa usa SEMPRE as DLLs corretas
    //  que estão junto com o .exe, em qualquer máquina.
    // --------------------------------------------------------
    WCHAR pastaExe[MAX_PATH];

    // Obtém o caminho completo do .exe em execução
    GetModuleFileNameW(NULL, pastaExe, MAX_PATH);

    // Remove o nome do arquivo, deixando só a pasta
    // Ex: "C:\projeto\detector_facial.exe" → "C:\projeto"
    PathRemoveFileSpecW(pastaExe);

    // Define essa pasta como prioridade para busca de DLLs
    SetDllDirectoryW(pastaExe);

    // --------------------------------------------------------

    std::cout << "Iniciando detector de rosto e olhos..." << std::endl;
    std::cout << "Pressione ESC para sair." << std::endl;


    // --------------------------------------------------------
    //  PASSO 1: Carregar os detectores (arquivos XML)
    // --------------------------------------------------------

    cv::CascadeClassifier detectorRosto;
    cv::CascadeClassifier detectorOlhos;

    if (!detectorRosto.load(ARQUIVO_ROSTO)) {
        std::cerr << "ERRO: Arquivo nao encontrado: " << ARQUIVO_ROSTO << std::endl;
        std::cerr << "Copie o arquivo .xml para a mesma pasta do programa." << std::endl;
        return 1;
    }

    if (!detectorOlhos.load(ARQUIVO_OLHOS)) {
        std::cerr << "ERRO: Arquivo nao encontrado: " << ARQUIVO_OLHOS << std::endl;
        std::cerr << "Copie o arquivo .xml para a mesma pasta do programa." << std::endl;
        return 1;
    }

    std::cout << "Detectores carregados com sucesso!" << std::endl;


    // --------------------------------------------------------
    //  PASSO 2: Abrir a câmera
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
    // --------------------------------------------------------

    cv::Mat frameColorido;
    cv::Mat frameCinza;

    while (true)
    {
        // -- a) Captura um frame da câmera -------------------
        camera >> frameColorido;

        if (frameColorido.empty()) {
            std::cerr << "AVISO: Frame vazio. Encerrando." << std::endl;
            break;
        }

        // Espelha a imagem (fica como olhar num espelho)
        cv::flip(frameColorido, frameColorido, 1);


        // -- b) Converte para escala de cinza ----------------
        cv::cvtColor(frameColorido, frameCinza, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(frameCinza, frameCinza);


        // -- c) Detectar rostos ------------------------------
        std::vector<cv::Rect> rostos;
        detectorRosto.detectMultiScale(
            frameCinza,
            rostos,
            1.1,
            5,
            0,
            cv::Size(80, 80)
        );


        // -- d) Para cada rosto encontrado -------------------
        for (const cv::Rect& rosto : rostos)
        {
            // Retângulo VERDE em volta do rosto
            cv::rectangle(frameColorido, rosto, COR_ROSTO, ESPESSURA_LINHA);

            // Recorta a região do rosto para buscar olhos
            cv::Mat regiaoRosto = frameCinza(rosto);

            // -- e) Detectar olhos dentro do rosto -----------
            std::vector<cv::Rect> olhos;
            detectorOlhos.detectMultiScale(
                regiaoRosto,
                olhos,
                1.1,
                4,
                0,
                cv::Size(20, 20)
            );

            int contadorOlhos = 0;

            for (const cv::Rect& olho : olhos)
            {
                if (contadorOlhos >= 2) break;

                // Converte coordenadas locais do olho
                // para coordenadas do frame completo
                cv::Rect olhoNoFrame(
                    rosto.x + olho.x,
                    rosto.y + olho.y,
                    olho.width,
                    olho.height
                );

                // Retângulo AZUL em volta do olho
                cv::rectangle(frameColorido, olhoNoFrame, COR_OLHOS, ESPESSURA_LINHA);

                contadorOlhos++;
            }
        }


        // -- f) Mostra informações na tela -------------------
        cv::putText(
            frameColorido,
            "Pressione ESC para sair",
            cv::Point(10, 25),
            cv::FONT_HERSHEY_SIMPLEX,
            0.6,
            cv::Scalar(200, 200, 200),
            1
        );

        std::string info = "Rostos: " + std::to_string(rostos.size());
        cv::putText(frameColorido, info, cv::Point(10, 50),
                    cv::FONT_HERSHEY_SIMPLEX, 0.6,
                    cv::Scalar(0, 255, 0), 1);

        // -- g) Exibe o frame --------------------------------
        cv::imshow("Detector de Rosto e Olhos", frameColorido);

        // -- h) ESC para sair --------------------------------
        if (cv::waitKey(1) == 27) {
            std::cout << "ESC pressionado. Encerrando..." << std::endl;
            break;
        }
    }


    // --------------------------------------------------------
    //  PASSO 4: Liberar recursos
    // --------------------------------------------------------
    camera.release();
    cv::destroyAllWindows();

    std::cout << "Programa encerrado." << std::endl;
    return 0;
}