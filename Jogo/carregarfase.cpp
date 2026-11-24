#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "raylib.h"
//g++ carregarfase.cpp -o teste.exe -lraylib -lopengl32 -lgdi32 -lwinmm

using namespace std;

// Dimensões exatas para imagem 640x360 com blocos de 20px
const int LINHAS = 18;
const int COLUNAS = 32;
const int TAM = 20; 

struct Personagem {
    int linha;
    int coluna;
    int energia;
    int vidas;
    int pontos;
};

// Função para carregar o mapa do ficheiro de texto
bool carregarFase(const string& nomeFicheiro, vector<vector<int>>& matriz) {
    ifstream ficheiro(nomeFicheiro);
    
    if (!ficheiro.is_open()) {
        cerr << "Erro: Nao foi possivel abrir o ficheiro " << nomeFicheiro << "!" << endl;
        return false;
    }

    matriz.resize(LINHAS, vector<int>(COLUNAS));
    string linhaTexto;
    int r = 0;

    while (getline(ficheiro, linhaTexto) && r < LINHAS) {
        if (linhaTexto.length() < COLUNAS) {
            cerr << "Erro: A linha " << r + 1 << " tem menos de 32 colunas!" << endl;
            return false;
        }

        for (int c = 0; c < COLUNAS; c++) {
            matriz[r][c] = linhaTexto[c] - '0';
        }
        r++;
    }

    ficheiro.close();

    if (r < LINHAS) {
        cerr << "Erro: O ficheiro tem menos de 18 linhas!" << endl;
        return false;
    }

    cout << "Fase carregada com sucesso: " << nomeFicheiro << "\n" << endl;
    return true;
}

// Desenha o cenário: imagem de fundo na fase 1 e blocos coloridos nas fases 2 e 3
void desenharCenario(const vector<vector<int>>& matriz, int faseAtual, Texture2D fundoFase1) {
    if (faseAtual == 1) {
        // Desenha a imagem 640x360 preenchendo toda a área do mapa
        DrawTexture(fundoFase1, 0, 0, WHITE);
    } else {
        for (int i = 0; i < LINHAS; i++) {
            for (int j = 0; j < COLUNAS; j++) {
                Color corPiso = RAYWHITE;
                Color corParede = DARKGRAY;

                if (faseAtual == 2) { 
                    // Fase 2: Monte Everest
                    corPiso = RAYWHITE;
                    corParede = SKYBLUE;
                } 
                else if (faseAtual == 3) { 
                    // Fase 3: Vulcão
                    corPiso = MAROON;
                    corParede = BLACK;
                }

                Color cor = (matriz[i][j] == 1) ? corParede : ((matriz[i][j] == 2) ? GREEN : corPiso);
                DrawRectangle(j * TAM, i * TAM, TAM, TAM, cor);
                DrawRectangleLines(j * TAM, i * TAM, TAM, TAM, LIGHTGRAY);
            }
        }
    }
}

// Movimenta a personagem com verificação de paredes e limites
void moverPersonagem(const vector<vector<int>>& matriz, Personagem &personagem, int dl, int dc) {
    int novaLinha = personagem.linha + dl;
    int novaColuna = personagem.coluna + dc;
    
    if (novaLinha < 0 || novaLinha >= LINHAS || novaColuna < 0 || novaColuna >= COLUNAS)
        return;

    if (matriz[novaLinha][novaColuna] == 1)
        return;

    personagem.linha = novaLinha;
    personagem.coluna = novaColuna;
    
    if (personagem.energia > 0)
        personagem.energia--;
}

int main() {
    vector<vector<int>> mapa;
    int faseAtual = 1;

    if (!carregarFase("fase1.txt", mapa)) {
        mapa.assign(LINHAS, vector<int>(COLUNAS, 0));
    }

    const int barraStatusAltura = 40;
    const int larguraEcra = COLUNAS * TAM; // 640 pixels
    const int alturaEcra = LINHAS * TAM + barraStatusAltura; // 360 + 40 = 400 pixels

    InitWindow(larguraEcra, alturaEcra, "Aventura em 3 Mundos - Jogo");
    SetTargetFPS(60);

    Texture2D texturaPersonagem = LoadTexture("personagem.png");
    Texture2D fundoFase1 = LoadTexture("fase1_bg.png");

    Personagem heroina;
    heroina.linha = 14; // Começa na altura da calçada da imagem
    heroina.coluna = 1;
    heroina.vidas = 3;
    heroina.energia = 100;
    heroina.pontos = 0;

    bool jogoConcluido = false;

    while (!WindowShouldClose()) {

        if (!jogoConcluido && heroina.vidas > 0 && heroina.energia > 0) {
            if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W))
                moverPersonagem(mapa, heroina, -1, 0);

            if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_S))
                moverPersonagem(mapa, heroina, 1, 0);

            if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_A))
                moverPersonagem(mapa, heroina, 0, -1);

            if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_D))
                moverPersonagem(mapa, heroina, 0, 1);

            // Verifica se chegou na saída (valor 2)
            if (mapa[heroina.linha][heroina.coluna] == 2) {
                heroina.pontos += 50;
                faseAtual++;

                if (faseAtual == 2) {
                    if (!carregarFase("fase2.txt", mapa)) {
                        mapa.assign(LINHAS, vector<int>(COLUNAS, 0));
                    }
                    heroina.linha = 1;
                    heroina.coluna = 1;
                    heroina.energia = 100; 
                } 
                else if (faseAtual == 3) {
                    if (!carregarFase("fase3.txt", mapa)) {
                        mapa.assign(LINHAS, vector<int>(COLUNAS, 0));
                    }
                    heroina.linha = 1;
                    heroina.coluna = 1;
                    heroina.energia = 100;
                } 
                else {
                    jogoConcluido = true;
                }
            }
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // Desenha o cenário atual
        desenharCenario(mapa, faseAtual, fundoFase1);

        // Define uma altura condizente com as portas do cenário (2.2 blocos = 44 pixels)
        float alturaDesenho = TAM * 2.2f; 
        
        // Calcula a largura mantendo a proporção original da imagem para não achatar
        float proporcao = (float)texturaPersonagem.width / (float)texturaPersonagem.height;
        float larguraDesenho = alturaDesenho * proporcao;

        // Ajusta a posição para os pés ficarem na base da linha atual e centralizados na coluna
        float posX = (float)(heroina.coluna * TAM) + (TAM - larguraDesenho) / 2.0f;
        float posY = (float)((heroina.linha + 1) * TAM) - alturaDesenho;

        Rectangle rectOrigem = { 0.0f, 0.0f, (float)texturaPersonagem.width, (float)texturaPersonagem.height };
        Rectangle rectDestino = { posX, posY, larguraDesenho, alturaDesenho };
        Vector2 origemTextura = { 0.0f, 0.0f };
        
        DrawTexturePro(texturaPersonagem, rectOrigem, rectDestino, origemTextura, 0.0f, WHITE);

        // Barra de status inferior
        DrawRectangle(0, LINHAS * TAM, larguraEcra, barraStatusAltura, DARKGRAY);
        DrawText(TextFormat("Fase: %d | Vidas: %d | Energia: %d | Pontos: %d", faseAtual, heroina.vidas, heroina.energia, heroina.pontos), 
                 10, LINHAS * TAM + 10, 20, WHITE);

        if (jogoConcluido) {
            DrawRectangle(larguraEcra / 2 - 150, 130, 300, 100, Fade(BLACK, 0.85f));
            DrawText("VOCE VENCEU O JOGO!", larguraEcra / 2 - 130, 165, 20, GREEN);
        }

        EndDrawing();
    }

    // Descarrega as texturas da memória antes de fechar
    UnloadTexture(texturaPersonagem);
    UnloadTexture(fundoFase1);

    CloseWindow();
    return 0;
}