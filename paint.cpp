/*
 * Computacao Grafica
 * Codigo Exemplo: Rasterizacao de Segmentos de Reta com GLUT/OpenGL
 * Autor: Prof. Laurindo de Sousa Britto Neto
 *
 * Aluno: Neemias Calebe Pereira Freire
 * 
 * Objetivo: Desenvolva um Editor Gráfico em C/C++, estilo o Paint da Microsoft, utilizando as bibliotecas
 * GLUT/OpenGL. Leia os comentários do programa “paint.cpp” e utilize o programa “paint.cpp” como referência.
 * 
 *	--Translação são os botoes 'wasd' do teclado com suas direções padrões.
 *  --Rotação são os botoes 'q' e 'e'
 *  --Reflexão os botões 'r' com reflexão no eixo y e 't' no eixo x.
 *  --Cisalhamento na tecla 'c'
 *  --Escala nas teclas '+' e '-' (bem auto-explicativo :D)
 *  	
 *	Essas transformações são feitas somente ultima forma desenhada na tela.
 *	
 *  Problemas conhecidos:
 *  --Cisalhamento e rotação de um retangulo realizam uma transformação inesperada (as vezes nem transforma nada)
 *  --Rotação desenfreada faz com que os objetos diminuam de tamanho até se tornarem somente um pixel. Provavelmente
 *	  não calculei o centroide de forma correta da forma.	
 *
 */

// Bibliotecas utilizadas pelo OpenGL
#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <forward_list>
#include "glut_text.h"

using namespace std;

// Variaveis Globais
#define ESC 27
#define UP 119      	 //Tecla w
#define DOWN 115		 //Tecla s
#define LEFT 97			 //Tecla a
#define RIGHT 100		 //Tecla d
#define ROTATE_E 113     //Tecla q
#define ROTATE_R 101   	 //Tecla e
#define ESCALE_PLUS 43 	 //Tecla +
#define ESCALE_MINUS 45  //Tecla -
#define REFLEX_Y 114     //Tecla r
#define REFLEX_X 116     //Tecla t
#define CIS 99           //Tecla c

//Constantes
#define TRANSLA 5
#define ANGULO 10 * 3.141592 / 180.0
#define ESCALA_PLUS 1.12
#define ESCALA_MINUS 0.88
#define CISALHAMENTO 0.2

string modeName;

//Enumeracao com os tipos de formas geometricas
enum tipo_forma {LIN = 1, TRI = 2, RET = 3, POL = 4, CIR = 5}; // Linha, Triangulo, Retangulo Poligono, Circulo

//Verifica se foi realizado o primeiro clique do mouse
bool click1 = false, click2 = false, click3 = false;

//Coordenadas da posicao atual do mouse
int m_x, m_y;

//Coordenadas do primeiro clique e do segundo clique do mouse
int x_1, y_1, x_2, y_2, x_3, y_3;

//Indica o tipo de forma geometrica ativa para desenhar
int modo = LIN;

//Largura e altura da janela
int width = 512, height = 512;

// Definicao de vertice
struct vertice
{
	int x;
	int y;
};

// Definicao das formas geometricas
struct forma
{
	int tipo;
	forward_list<vertice> v; //lista encadeada de vertices
};

// Lista encadeada de formas geometricas
forward_list<forma> formas;
forward_list<vertice> poligonos;

// Funcao para armazenar uma forma geometrica na lista de formas
// Armazena sempre no inicio da lista
void pushForma(int tipo)
{
	forma f;
	f.tipo = tipo;
	formas.push_front(f);
}

// Funcao para armazenar um vertice na forma do inicio da lista de formas geometricas
// Armazena sempre no inicio da lista
void pushVertice(int x, int y)
{
	vertice v;
	v.x = x;
	v.y = y;
	formas.front().v.push_front(v);
}

//Fucao para armazenar uma Linha na lista de formas geometricas
void pushLinha(int x1, int y1, int x2, int y2)
{
	pushForma(LIN);
	pushVertice(x1, y1);
	pushVertice(x2, y2);
}

void pushQuadrilatero(int xa, int ya, int xb, int yb)
{
	pushForma(RET);
	pushVertice(xa, ya);
	pushVertice(xb, yb);
}

void pushTriangulo(int xa, int ya, int xb, int yb, int xc, int yc)
{
	pushForma(TRI);
	pushVertice(xa, ya);
	pushVertice(xb, yb);
	pushVertice(xc, yc);
}

void pushCirculo(int xa, int ya, int xb, int yb)
{
	pushForma(CIR);
	pushVertice(xa, ya);
	pushVertice(xb, yb);
}

void pushPoligono() {
    pushForma(POL);
    for (auto it = poligonos.begin(); it != poligonos.end(); ++it) {
        pushVertice(it->x, it->y);
    }
    poligonos.clear();
}

/*
 * Declaracoes antecipadas (forward) das funcoes (assinaturas das funcoes)
 */
void init(void);
void reshape(int w, int h);
void display(void);
void menu_popup(int value);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void mousePassiveMotion(int x, int y);
void drawPixel(int x, int y);
// Funcao que percorre a lista de formas geometricas, desenhando-as na tela
void drawFormas();
void bresenham(int xa, int ya, int xb, int yb);
void drawQuadrilatero(int xa, int ya, int xb, int yb);
void drawTriangulo(int xa, int ya, int xb, int yb, int xc, int yc);
void drawCirculo(int xm, int ym, int r);
void drawPoligono(int *xa, int *ya, int i);
void translacao(int dx, int dy);
void rotacao(float angulo);
void escala(float sx, float sy);
void reflexao(bool x, bool y);
void cisalhamento(float c);


/*
 * Funcao principal
 */
int main(int argc, char** argv)
{
	glutInit(&argc, argv); // Passagens de parametro C para o glut
	glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB); //Selecao do Modo do Display e do Sistema de cor
	glutInitWindowSize (width, height);  // Tamanho da janela do OpenGL
	glutInitWindowPosition (100, 100); //Posicao inicial da janela do OpenGL
	glutCreateWindow ("Computacao Grafica: Paint"); // Da nome para uma janela OpenGL
	init(); // Chama funcao init();
	glutReshapeFunc(reshape); //funcao callback para redesenhar a tela
	glutKeyboardFunc(keyboard); //funcao callback do teclado
	glutMouseFunc(mouse); //funcao callback do mouse
	glutPassiveMotionFunc(mousePassiveMotion); //fucao callback do movimento passivo do mouse
	glutDisplayFunc(display); //funcao callback de desenho

	// Define o menu pop-up
	glutCreateMenu(menu_popup);
	glutAddMenuEntry("Linha", LIN);
	glutAddMenuEntry("Retangulo", RET);
	glutAddMenuEntry("Triangulo", TRI);
	glutAddMenuEntry("Circulo", CIR);
	glutAddMenuEntry("Poligono", POL);
	glutAddMenuEntry("Sair", 0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);


	glutMainLoop(); // executa o loop do OpenGL
	return EXIT_SUCCESS; // retorna 0 para o tipo inteiro da funcao main();
}

/*
 * Inicializa alguns parametros do GLUT
 */
void init(void)
{
	glClearColor(1.0, 1.0, 1.0, 1.0); //Limpa a tela com a cor branca;
}

/*
 * Ajusta a projecao para o redesenho da janela
 */
void reshape(int w, int h)
{
	// Muda para o modo de projecao e reinicializa o sistema de coordenadas
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// Definindo o Viewport para o tamanho da janela
	glViewport(0, 0, w, h);

	width = w;
	height = h;
	glOrtho (0, w, 0, h, -1 , 1);

	// muda para o modo de desenho
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

}

/*
 * Controla os desenhos na tela
 */
void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT); //Limpa o buffer de cores e reinicia a matriz
	glColor3f (0.0, 0.0, 0.0); // Seleciona a cor default como preto
	drawFormas(); // Desenha as formas geometricas da lista
	//Desenha texto com as coordenadas da posicao do mouse
	draw_text_stroke(0, 5, "(x:" + to_string(m_x) + ", y:" + to_string(m_y) + ")", 0.2);

	if(modo == 1) modeName = "Linha";
	if(modo == 2) modeName = "Triangulo";
	if(modo == 3) modeName = "Retangulo";
	if(modo == 4) modeName = "Poligono";
	if(modo == 5) modeName = "Circulo";
	
	draw_text_stroke(0, height - 25, "Modo:" + modeName, 0.2);

	glutSwapBuffers(); // manda o OpenGl renderizar as primitivas

}

/*
 * Controla o menu pop-up
 */
void menu_popup(int value)
{
	if (value == 0) exit(EXIT_SUCCESS);
	modo = value;
}

/*
 * Controle das teclas comuns do teclado
 * Os controles são o wasd do teclado com suas direções padroes 
 */
void keyboard(unsigned char key, int x, int y)
{
	switch (key)   // key - variavel que possui valor ASCII da tecla precionada
	{
		case ESC:
        {			
			exit(EXIT_SUCCESS);
			break;
	    }
	    case UP:
	    {
			translacao(0, TRANSLA);
			glutPostRedisplay();
			break;
		}
		case DOWN:
	    {
			translacao(0,-TRANSLA);
			glutPostRedisplay();
			break;
		}
		case LEFT:
	    {
			translacao(-TRANSLA,0);
			glutPostRedisplay();
			break;
		}
		case RIGHT:
	    {
			translacao(TRANSLA,0);
			glutPostRedisplay();
			break;
		}
		case ROTATE_E:
		{	
			rotacao(ANGULO);
			glutPostRedisplay();
			break;
		}
		case ROTATE_R:
		{	
			rotacao(-ANGULO);
			glutPostRedisplay();
			break;
		}
		case ESCALE_PLUS:
		{	
			escala(ESCALA_PLUS, ESCALA_PLUS);
			glutPostRedisplay();
			break;
		}
		case ESCALE_MINUS:
		{	
			escala(ESCALA_MINUS, ESCALA_MINUS);
			glutPostRedisplay();
			break;
		}
		case REFLEX_Y:
		{
			reflexao(false, true);
			glutPostRedisplay();
			break;
		}
		case REFLEX_X:
		{
			reflexao(true, false);
			glutPostRedisplay();
			break;
		}
		case CIS:{
			cisalhamento(CISALHAMENTO);
			glutPostRedisplay();
			break;
		}
		case 32: {
			pushPoligono();
			poligonos.clear();
			glutPostRedisplay();
			break;
		}



	}
}

void mouse(int button, int state, int x, int y){
    switch (button) {
        case GLUT_LEFT_BUTTON:
            switch(modo){
                case LIN:{
                    if (state == GLUT_DOWN) {
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            pushLinha(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
                	break;
				}
				case RET:{
					if (state == GLUT_DOWN) {
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            pushQuadrilatero(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
					break;
				}
				case TRI:{
					if (state == GLUT_DOWN) {
						click1 = true;
						
						if(click3)
						{
							x_3 = x;
							y_3 = height - y - 1;
							
							pushTriangulo(x_1, y_1, x_2, y_2, x_3, y_3);
							
							click1 = false;
							click2 = false;
							click2 = false;
							glutPostRedisplay();
						}
						if (click2 && !click3)
						{
							click3 = true;
							
							x_2 = x;
							y_2 = height - y - 1;
						}
						if (click1 && !click2)
						{
							click2 = true;
							
							x_1 = x;
							y_1 = height - y;
							x_2 = x_1;
							y_2 = y_1;
						}	
                    }
					break;
				}
				case CIR:{
					if (state == GLUT_DOWN) {
                        if(click1){
                            x_2 = x;
                            y_2 = height - y - 1;
                            pushCirculo(x_1, y_1, x_2, y_2);
                            click1 = false;
                            glutPostRedisplay();
                        }else{
                            click1 = true;
                            x_1 = x;
                            y_1 = height - y - 1;
                        }
                    }
				}
				case POL:{
					if(state == GLUT_DOWN){
						vertice v;
						v.x = x;
						v.y = y;
						
						poligonos.push_front(v);
					}
				}
            }
        break;

//        case GLUT_MIDDLE_BUTTON:
//            if (state == GLUT_DOWN) {
//                glutPostRedisplay();
//            }
//        break;
//
//        case GLUT_RIGHT_BUTTON:
//            if (state == GLUT_DOWN) {
//                glutPostRedisplay();
//            }
//        break;
            
    }
}


/*
 * Controle da posicao do cursor do mouse
 */
void mousePassiveMotion(int x, int y)
{
	m_x = x;
	m_y = height - y - 1;
	glutPostRedisplay();
}

/*
 * Funcao para desenhar apenas um pixel na tela
 */
void drawPixel(int x, int y)
{
	glBegin(GL_POINTS); // Seleciona a primitiva GL_POINTS para desenhar
	glVertex2i(x, y);
	glEnd();  // indica o fim do ponto
}

/*
 *Funcao que desenha a lista de formas geometricas
 */
void drawFormas()
{	int raio;
	//Apos o primeiro clique, desenha a reta com a posicao atual do mouse
	if(click1)
	{
		if (modo == LIN) bresenham(x_1, y_1, m_x, m_y);
		if (modo == RET) drawQuadrilatero(x_1, y_1, m_x, m_y);
		if (modo == TRI) drawTriangulo(x_1, y_1, m_x, m_y, x_2, y_2);
		
		if (modo == CIR){
			raio = sqrt(pow(x_1 - m_x, 2) + pow(y_1 - m_y, 2));
			drawCirculo(x_1, y_1, raio);
		} 
			
	}

	//Percorre a lista de formas geometricas para desenhar
	for(forward_list<forma>::iterator f = formas.begin(); f != formas.end(); f++)
	{
		switch (f->tipo)
		{
		case LIN:
		{
			int i = 0, x[2], y[2];
			//Percorre a lista de vertices da forma linha para desenhar
			for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++)
			{
				x[i] = v->x;
				y[i] = v->y;
			}
			//Desenha o segmento de reta apos dois cliques
			bresenham(x[0], y[0], x[1], y[1]);

			break;
		}
		case RET:
		{
			int i = 0, x[2], y[2];
			//Percorre a lista de vertices da forma linha para desenhar
			for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++)
			{
				x[i] = v->x;
				y[i] = v->y;
			}
			//Desenha o segmento de reta apos dois cliques
			drawQuadrilatero(x[0], y[0], x[1], y[1]);

			break;
		}
		case TRI:
		{
			int i = 0, x[3], y[3];
			//Percorre a lista de vertices da forma linha para desenhar
			for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++)
			{
				x[i] = v->x;
				y[i] = v->y;
			}
			//Desenha o segmento de reta apos dois cliques
			drawTriangulo(x[0], y[0], x[1], y[1], x[2], y[2]);

			break;
		}
		case CIR:
		{
			int i = 0, x[3], y[3];
			//Percorre a lista de vertices da forma linha para desenhar
			for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++)
			{
				x[i] = v->x;
				y[i] = v->y;
			}
			//Distancia euclidiana
			raio = sqrt(pow(x[0] - x[1], 2) + pow(y[0] - y[1], 2));
			drawCirculo(x[1], y[1], raio);

			break;
		}
		case POL:
		{
			int i = 0, x[500], y[500];
			
			//Percorre a lista de vertices da forma linha para desenhar
			for(forward_list<vertice>::iterator v = f->v.begin(); v != f->v.end(); v++, i++)
			{
				x[i] = v->x;
				y[i] = v->y;
			}
			//Desenha o segmento de reta apos dois cliques
			drawPoligono(x, y, i);

			break;
		}
		}
	}
}

void drawQuadrilatero(int xa, int ya, int xb, int yb)
{
	//Reta A
	bresenham(xa, ya, xb, ya);
	//Reta B
	bresenham(xb, ya, xb, yb);
	//Reta C
	bresenham(xb, yb, xa, yb);
	//Reta D
	bresenham(xa, yb, xa, ya);
}

void drawTriangulo(int xa, int ya, int xb, int yb, int xc, int yc)
{
	//Lado A
	bresenham(xa, ya, xb, yb);
	//Lado B
	bresenham(xb, yb, xc, yc);
	//Lado C
	bresenham(xc, yc, xa, ya);
}

void drawPoligono(int *xa, int *ya, int i)
{
	for(int j; j < i; j++)
	{
		bresenham(xa[j], ya[j], xa[j+1], ya[j+1]);
	}
	bresenham(xa[i], ya[i], xa[0], ya[0]);
}

void drawCirculo(int xm, int ym, int r)
{
	int d = 1 - r;
	int delta_e = 3;
	int delta_se = -2*r + 5;
	int xi = 0, yi = r;
	
	drawPixel(xm - xi, ym + yi);
	drawPixel(xm - yi, ym - xi); 
	drawPixel(xm + xi, ym - yi); 
	drawPixel(xm + yi, ym + xi); 
	
	while(yi > xi)
	{
		
		if (d < 0)
		{
			d += delta_e;
			delta_e += 2;
			delta_se += 2;
		}
		else
		{
			d += delta_se;
			delta_e += 2;
			delta_se += 4;
			yi--;
		}
		xi += 1;
		
		drawPixel(xm + xi, ym + yi);
		drawPixel(xm - xi, ym + yi);
		drawPixel(xm + xi, ym - yi);
		drawPixel(xm - xi, ym - yi);
		drawPixel(xm + yi, ym + xi);
		drawPixel(xm - yi, ym + xi);
		drawPixel(xm + yi, ym - xi);
		drawPixel(xm - yi, ym - xi);
	
	}
	
}

void translacao(int dx = 0, int dy = 0)
{
	for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it)
	{
		it->x += dx;
		it->y += dy;
	}
}

void rotacao(float angulo)
{
	float xc = 0, yc = 0;
	float angulo_cos = cos(angulo);
	float angulo_sen = sin(angulo);
	float dx, dy, dx_1, dy_1;
	int tam = 0;
	
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc += it->x;
        yc += it->y;
        tam++;
    }
    
    if (tam == 0) return;
    
	xc /= tam;
    yc /= tam;

    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        it->x -= xc;
        it->y -= yc;
    }
	
	for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it)
	{
		dx = it->x;
		dy = it->y;
		
		dx_1 = angulo_cos * dx - angulo_sen * dy;
		dy_1 = angulo_sen * dx + angulo_cos * dy;
		
		it->x = dx_1;
		it->y = dy_1;
	}
	
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        it->x += xc;
        it->y += yc;
    }
}

void escala(float sx, float sy)
{
    float xc = 0, yc = 0;
    int tam = 0;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc += it->x;
        yc += it->y;
        tam++;
    }
    xc /= tam;
    yc /= tam;

    // aplicar a escala
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        it->x *= sx;
        it->y *= sy;
    }

    // calcular o centroide depois da escala
    float xc_novo = 0, yc_novo = 0;
    tam = 0;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc_novo += it->x;
        yc_novo += it->y;
        tam++;
    }
    xc_novo /= tam;
    yc_novo /= tam;

    // ajustar as coordenadas para manter o centroide na mesma posição
    float dx = xc - xc_novo;
    float dy = yc - yc_novo;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        it->x += dx;
        it->y += dy;
    }
}

void reflexao(bool x, bool y)
{
	float xc = 0, yc = 0;
    int tam = 0;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc += it->x;
        yc += it->y;
        tam++;
    }
    xc /= tam;
    yc /= tam;
    
	if (x)
	{
	    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it)
		{
			it->x *= -1;
		}
	}
	else if(y)
	{
		for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it)
		{
			it->y *= -1;
		}	
	}
	else
	{
	    return;	
	}
	
	float xc_novo = 0, yc_novo = 0;
    tam = 0;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc_novo += it->x;
        yc_novo += it->y;
        tam++;
    }
    
    xc_novo /= tam;
    yc_novo /= tam;

    // ajustar as coordenadas para manter o centroide na mesma posição
    float dx = xc - xc_novo;
    float dy = yc - yc_novo;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        it->x += dx;
        it->y += dy;
    }
}

void cisalhamento(float c) 
{
    float x, y;
    
    float matriz[2][2] = {
        {1.0f, c},
        {0.0f, 1.0f}
    };
    
    float xc = 0, yc = 0;
    int tam = 0;
    
	for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc += it->x;
        yc += it->y;
        tam++;
    }
    xc /= tam;
    yc /= tam;
    
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        x = it->x;
        y = it->y;

        it->x = matriz[0][0] * x + matriz[0][1] * y;
        it->y = matriz[1][0] * x + matriz[1][1] * y;
    }
    
    float xc_novo = 0, yc_novo = 0;
    tam = 0;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        xc_novo += it->x;
        yc_novo += it->y;
        tam++;
    }
    
    xc_novo /= tam;
    yc_novo /= tam;

    // ajustar as coordenadas para manter o centroide na mesma posição
    float dx = xc - xc_novo;
    float dy = yc - yc_novo;
    for (auto it = formas.front().v.begin(); it != formas.front().v.end(); ++it) {
        it->x += dx;
        it->y += dy;
    }
}

/*
	Tentei implementar da forma que esta nos slides, mas a unica que funcionou foi
	essa implementação no site: http://members.chello.at/~easyfilter/Bresenham.pdf
*/
void bresenham(int xa, int ya, int xb, int yb)
{
	int dx = abs(xb - xa);
	int dy = -abs(yb - ya);
	int sx = (xa < xb) ? 1 : -1;
	int sy = (ya < yb) ? 1 : -1;

	int err = dx + dy;
	int e2;

	for (;;)
	{
		drawPixel(xa, ya);
		e2 = 2 * err;

		if (e2 >= dy)
		{
			if (xa == xb) break;
			err += dy;
			xa += sx;
		}
		if (e2 <= dx)
		{
			if (ya == yb) break;
			err += dx;
			ya += sy;
		}

	}
}