// ============================================================
//  DETECTOR DE ROSTO E OLHOS + CONTROLE DO MOUSE
// ============================================================

#include <windows.h>
#include <shlwapi.h>
#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>
#include <deque>
#include <numeric>
#include <csignal>    // necessário para capturar Ctrl+C


// ============================================================
//  FLAG GLOBAL DE CONTROLE DO LOOP
//
//  Quando o usuário pressiona Ctrl+C ou fecha a janela,
//  essa variável vira false e o loop principal encerra
//  de forma limpa, liberando câmera e janelas.
//
//  "volatile" diz ao compilador que essa variável pode
//  mudar a qualquer momento (pelo sinal do Ctrl+C),
//  impedindo otimizações que ignorariam a mudança.
// ============================================================
volatile bool rodando = true;


// ============================================================
//  FUNÇÃO: aoReceberCtrlC
//
//  É chamada automaticamente pelo sistema operacional
//  quando o usuário pressiona Ctrl+C.
//  Apenas muda a flag para false — o loop principal
//  detecta isso e encerra com limpeza.
// ============================================================
void aoReceberCtrlC(int)
{
    std::cout << "\n[INFO] Ctrl+C detectado. Encerrando..." << std::endl;
    rodando = false;
}


// ============================================================
//  CONFIGURAÇÕES
// ============================================================

const int CAMERA_INDEX = 0;

const std::string ARQUIVO_ROSTO = "haarcascade_frontalface_default.xml";
const std::string ARQUIVO_OLHOS = "haarcascade_eye.xml";

const cv::Scalar COR_ROSTO(0, 255, 0);
const cv::Scalar COR_OLHOS(255, 0, 0);
const cv::Scalar COR_CENTRO(0, 255, 255);
const int ESPESSURA_LINHA = 2;

const int    SUAVIZACAO   = 8;
const double SENSIBILIDADE = 3.0;


// ============================================================
//  CLASSE: FiltroDeSuavizacao
// ============================================================
class FiltroDeSuavizacao {
public:
    FiltroDeSuavizacao(int tamanho) : m_tamanho(tamanho) {}

    cv::Point atualizar(int x, int y)
    {
        m_filax.push_back(x);
        m_filay.push_back(y);

        if ((int)m_filax.size() > m_tamanho) {
            m_filax.pop_front();
            m_filay.pop_front();
        }

        int somaX = std::accumulate(m_filax.begin(), m_filax.end(), 0);
        int somaY = std::accumulate(m_filay.begin(), m_filay.end(), 0);
        int n     = (int)m_filax.size();

        return cv::Point(somaX / n, somaY / n);
    }

private:
    int             m_tamanho;
    std::deque<int> m_filax;
    std::deque<int> m_filay;
};


// ============================================================
//  FUNÇÃO: mapearValor
// ============================================================
int mapearValor(int valor,
                int entradaMin, int entradaMax,
                int saidaMin,   int saidaMax)
{
    if (entradaMax == entradaMin) return saidaMin;

    double proporcao = (double)(valor - entradaMin) /
                       (double)(entradaMax - entradaMin);

    int resultado = saidaMin + (int)(proporcao * (saidaMax - saidaMin));

    if (resultado < saidaMin) resultado = saidaMin;
    if (resultado > saidaMax) resultado = saidaMax;

    return resultado;
}


// ============================================================
//  FUNÇÃO: desenharBarraDeStatus
// ============================================================
void desenharBarraDeStatus(cv::Mat& frame,
                           const std::string& mensagem,
                           cv::Scalar corTexto)
{
    int altura  = frame.rows;
    int largura = frame.cols;

    cv::rectangle(frame,
                  cv::Point(0, altura - 35),
                  cv::Point(largura, altura),
                  cv::Scalar(0, 0, 0), cv::FILLED);

    cv::putText(frame, mensagem,
                cv::Point(10, altura - 12),
                cv::FONT_HERSHEY_SIMPLEX, 0.5,
                corTexto, 1);
}


// ============================================================
//  FUNÇÃO PRINCIPAL
// ============================================================
int main()
{
    // --------------------------------------------------------
    //  Registra a função que trata Ctrl+C
    //
    //  signal(SIGINT, aoReceberCtrlC) diz ao sistema:
    //  "quando receber Ctrl+C, chame aoReceberCtrlC()"
    //  SIGINT = sinal de interrupção (Ctrl+C)
    // --------------------------------------------------------
    signal(SIGINT,  aoReceberCtrlC);
    signal(SIGTERM, aoReceberCtrlC);  // também trata o "finalizar processo"


    // --------------------------------------------------------
    //  Força DLLs da pasta do .exe
    // --------------------------------------------------------
    WCHAR pastaExe[MAX_PATH];
    GetModuleFileNameW(NULL, pastaExe, MAX_PATH);
    PathRemoveFileSpecW(pastaExe);
    SetDllDirectoryW(pastaExe);


    // --------------------------------------------------------
    //  Resolução da tela
    // --------------------------------------------------------
    const int LARGURA_TELA = GetSystemMetrics(SM_CXSCREEN);
    const int ALTURA_TELA  = GetSystemMetrics(SM_CYSCREEN);

    std::cout << "==========================================" << std::endl;
    std::cout << "  Detector de Rosto + Controle do Mouse  " << std::endl;
    std::cout << "==========================================" << std::endl;
    std::cout << "[INFO] Resolucao da tela: "
              << LARGURA_TELA << " x " << ALTURA_TELA << std::endl;
    std::cout << "[INFO] Para sair: ESC, tecla Q, botao X da janela ou Ctrl+C"
              << std::endl;


    // --------------------------------------------------------
    //  Carrega detectores
    // --------------------------------------------------------
    std::cout << "[INFO] Carregando detectores..." << std::endl;

    cv::CascadeClassifier detectorRosto;
    cv::CascadeClassifier detectorOlhos;

    if (!detectorRosto.load(ARQUIVO_ROSTO)) {
        std::cerr << "[ERRO] Arquivo nao encontrado: " << ARQUIVO_ROSTO << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[OK]   Detector de rosto carregado." << std::endl;

    if (!detectorOlhos.load(ARQUIVO_OLHOS)) {
        std::cerr << "[ERRO] Arquivo nao encontrado: " << ARQUIVO_OLHOS << std::endl;
        system("pause");
        return 1;
    }
    std::cout << "[OK]   Detector de olhos carregado." << std::endl;


    // --------------------------------------------------------
    //  Abre a câmera
    // --------------------------------------------------------
    std::cout << "[INFO] Abrindo camera " << CAMERA_INDEX << "..." << std::endl;

    cv::VideoCapture camera(CAMERA_INDEX);

    if (!camera.isOpened()) {
        std::cerr << "[ERRO] Nao foi possivel abrir a camera." << std::endl;
        std::cerr << "[ERRO] Verifique se ela esta conectada e nao esta"
                     " em uso (Teams, Zoom, etc)." << std::endl;
        system("pause");
        return 1;
    }

    const int LARGURA_CAMERA = (int)camera.get(cv::CAP_PROP_FRAME_WIDTH);
    const int ALTURA_CAMERA  = (int)camera.get(cv::CAP_PROP_FRAME_HEIGHT);

    std::cout << "[OK]   Camera aberta. Resolucao: "
              << LARGURA_CAMERA << " x " << ALTURA_CAMERA << std::endl;


    // --------------------------------------------------------
    //  Cria a janela de preview
    // --------------------------------------------------------
    const std::string NOME_JANELA = "Detector de Rosto | Q ou ESC para sair";

    cv::namedWindow(NOME_JANELA, cv::WINDOW_AUTOSIZE);
    cv::moveWindow(NOME_JANELA, 50, 50);

    std::cout << "[OK]   Janela de preview criada." << std::endl;
    std::cout << "[INFO] Sensibilidade: " << SENSIBILIDADE << "x" << std::endl;
    std::cout << "------------------------------------------" << std::endl;


    // --------------------------------------------------------
    //  Calcula zona ativa da câmera (sensibilidade)
    // --------------------------------------------------------
    int centroX     = LARGURA_CAMERA / 2;
    int centroY     = ALTURA_CAMERA  / 2;
    int zonaX       = (int)(LARGURA_CAMERA / (2.0 * SENSIBILIDADE));
    int zonaY       = (int)(ALTURA_CAMERA  / (2.0 * SENSIBILIDADE));
    int entradaMinX = centroX - zonaX;
    int entradaMaxX = centroX + zonaX;
    int entradaMinY = centroY - zonaY;
    int entradaMaxY = centroY + zonaY;


    // --------------------------------------------------------
    //  Filtro de suavização
    // --------------------------------------------------------
    FiltroDeSuavizacao filtro(SUAVIZACAO);


    // --------------------------------------------------------
    //  Loop principal
    // --------------------------------------------------------
    cv::Mat frameColorido;
    cv::Mat frameCinza;
    bool rostoDetectadoAntes = false;

    while (rodando)   // ← verifica a flag a cada iteração
    {
        // Captura frame
        camera >> frameColorido;
        if (frameColorido.empty()) {
            std::cerr << "[AVISO] Frame vazio. Encerrando." << std::endl;
            break;
        }

        cv::flip(frameColorido, frameColorido, 1);
        cv::cvtColor(frameColorido, frameCinza, cv::COLOR_BGR2GRAY);
        cv::equalizeHist(frameCinza, frameCinza);

        // Detecta rostos
        std::vector<cv::Rect> rostos;
        detectorRosto.detectMultiScale(
            frameCinza, rostos,
            1.1, 5, 0, cv::Size(80, 80)
        );

        if (rostos.empty())
        {
            desenharBarraDeStatus(frameColorido,
                "Buscando rosto...  Posicione o rosto no centro da camera",
                cv::Scalar(0, 200, 255));

            if (rostoDetectadoAntes) {
                std::cout << "[AVISO] Rosto perdido. Reposicione." << std::endl;
                rostoDetectadoAntes = false;
            }
        }
        else
        {
            // Maior rosto detectado
            cv::Rect rostoMaior = rostos[0];
            for (const cv::Rect& r : rostos) {
                if (r.area() > rostoMaior.area()) rostoMaior = r;
            }

            if (!rostoDetectadoAntes) {
                std::cout << "[OK]   Rosto detectado. Controlando cursor." << std::endl;
                rostoDetectadoAntes = true;
            }

            cv::rectangle(frameColorido, rostoMaior, COR_ROSTO, ESPESSURA_LINHA);

            int centroRostoX = rostoMaior.x + rostoMaior.width  / 2;
            int centroRostoY = rostoMaior.y + rostoMaior.height / 2;

            cv::circle(frameColorido,
                       cv::Point(centroRostoX, centroRostoY),
                       5, COR_CENTRO, -1);

            cv::Point pos = filtro.atualizar(centroRostoX, centroRostoY);

            int cursorX = mapearValor(pos.x,
                                      entradaMinX, entradaMaxX,
                                      0, LARGURA_TELA);
            int cursorY = mapearValor(pos.y,
                                      entradaMinY, entradaMaxY,
                                      0, ALTURA_TELA);

            SetCursorPos(cursorX, cursorY);

            // Zona ativa (retângulo cinza)
            cv::rectangle(frameColorido,
                          cv::Point(entradaMinX, entradaMinY),
                          cv::Point(entradaMaxX, entradaMaxY),
                          cv::Scalar(128, 128, 128), 1);

            // Detecta olhos
            cv::Mat regiaoRosto = frameCinza(rostoMaior);
            std::vector<cv::Rect> olhos;
            detectorOlhos.detectMultiScale(
                regiaoRosto, olhos,
                1.1, 4, 0, cv::Size(20, 20)
            );

            int contadorOlhos = 0;
            for (const cv::Rect& olho : olhos) {
                if (contadorOlhos >= 2) break;
                cv::Rect olhoNoFrame(
                    rostoMaior.x + olho.x,
                    rostoMaior.y + olho.y,
                    olho.width, olho.height
                );
                cv::rectangle(frameColorido, olhoNoFrame,
                              COR_OLHOS, ESPESSURA_LINHA);
                contadorOlhos++;
            }

            std::string info =
                "Rosto: (" + std::to_string(centroRostoX) +
                ", " + std::to_string(centroRostoY) + ")" +
                "  Cursor: (" + std::to_string(cursorX) +
                ", " + std::to_string(cursorY) + ")" +
                "  Sens: " + std::to_string((int)SENSIBILIDADE) + "x";

            desenharBarraDeStatus(frameColorido, info, cv::Scalar(0, 255, 0));
        }

        // Faixa superior
        cv::rectangle(frameColorido,
                      cv::Point(0, 0),
                      cv::Point(frameColorido.cols, 30),
                      cv::Scalar(0, 0, 0), cv::FILLED);

        cv::putText(frameColorido,
                    "Zona cinza = area ativa | Q ou ESC = sair",
                    cv::Point(10, 20),
                    cv::FONT_HERSHEY_SIMPLEX, 0.5,
                    cv::Scalar(200, 200, 200), 1);

        // Exibe o frame
        cv::imshow(NOME_JANELA, frameColorido);


        // ----------------------------------------------------
        //  Verifica teclas E fechamento da janela
        //
        //  cv::waitKey(1): espera 1ms e retorna o código
        //  da tecla pressionada. O "& 0xFF" isola apenas
        //  os 8 bits menos significativos, o que garante
        //  compatibilidade entre sistemas operacionais.
        //
        //  Aceitamos ESC (código 27) e Q (código 'q' ou 'Q')
        //  para que o usuário possa sair mesmo sem foco
        //  exato na janela (Q é mais fácil de pressionar).
        // ----------------------------------------------------
        int tecla = cv::waitKey(1) & 0xFF;

        if (tecla == 27 || tecla == 'q' || tecla == 'Q') {
            std::cout << "[INFO] Tecla de saida pressionada. Encerrando..."
                      << std::endl;
            rodando = false;
        }

        // ----------------------------------------------------
        //  Verifica se o botão X da janela foi clicado
        //
        //  getWindowProperty retorna -1 se a janela foi
        //  fechada pelo botão X. Isso evita o programa
        //  continuar rodando sem janela visível.
        // ----------------------------------------------------
        double visivel = cv::getWindowProperty(
            NOME_JANELA, cv::WND_PROP_VISIBLE
        );

        if (visivel < 1) {
            std::cout << "[INFO] Janela fechada pelo usuario. Encerrando..."
                      << std::endl;
            rodando = false;
        }

    } // fim do loop


    // --------------------------------------------------------
    //  Limpeza de recursos
    //
    //  Sempre executado — seja por ESC, Q, X ou Ctrl+C.
    //  Libera a câmera e fecha todas as janelas do OpenCV.
    // --------------------------------------------------------
    std::cout << "[INFO] Liberando recursos..." << std::endl;
    camera.release();
    cv::destroyAllWindows();
    std::cout << "[OK]   Programa encerrado com sucesso." << std::endl;

    return 0;
}