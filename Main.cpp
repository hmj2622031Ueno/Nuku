#include "DxLib.h"
#include<stdlib.h>

const int WIDTH = 1000, HEIGHT = 700;

const int GRASS_NUM = 5;
const int GRASS_SIZE = 200;

int grassX[GRASS_NUM];
int grassY[GRASS_NUM];
int grassType[GRASS_NUM];

bool overlap;

int LoadGraphWithCheck(const char* file);

int LoadGraphWithCheck(const char* file)
{
	int res = LoadGraph(file);
	if (res == -1) { MessageBox(GetMainWindowHandle(), file, "画像読み込みに失敗", MB_OK | MB_ICONSTOP); }
	return res;
}

int APIENTRY WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nCmdShow)
{
	SetWindowText("抜くゲーム");	// ウィンドウのタイトル
	SetGraphMode(WIDTH, HEIGHT, 32);	// ウィンドウの大きさとカラービット数の指定
	ChangeWindowMode(true);	// ウィンドウモードで起動
	if (DxLib_Init() == -1) return -1;	// ライブラリ初期化　エラーが起きたら終了
	SetBackgroundColor(0, 0, 0);	// 背景色の指定
	SetDrawScreen(DX_SCREEN_BACK);	// 描画面を裏画面にする

	// 画像読み込み
	int imgGround = LoadGraphWithCheck("image/ground.png");
	int imgGrass[3] =
	{
		LoadGraphWithCheck("image/smallGrass.png"),
		LoadGraphWithCheck("image/bigGrass.png"),
		LoadGraphWithCheck("image/goldGrass.png")
	};

	// 草をランダムな位置に配置
	for (int i = 0; i < GRASS_NUM; i++)
	{
		while (1)
		{
			int x = GetRand(WIDTH - GRASS_SIZE);
			int y = GetRand(HEIGHT - GRASS_SIZE);

			overlap = false;
		}
		//grassX[i] = GetRand(WIDTH - 200);
		//grassY[i] = GetRand(HEIGHT - 200);
		//grassType[i] = GetRand(2);
	}


	while (1)
	{
		ClearDrawScreen();	// 画面をクリアする
		DrawGraph(0, 0, imgGround, false);

		for (int i = 0; i < GRASS_NUM; i++)
		{
			DrawGraph(grassX[i], grassY[i], imgGrass[grassType[i]], true);
		}

		ScreenFlip();	// 裏画面の内容を表画面に反映させる
		WaitTimer(16);	// 一定時間待つ
		if (ProcessMessage() == -1) { break; }	// Windows から情報を受け取りエラーが起きたら終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) { break; }	// ESCキーが押されたら終了
	}
	DxLib_End();	// DXライブラリ使用の終了処理
	return 0;
}