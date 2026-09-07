#include "DxLib.h"
#include<stdlib.h>

const int WIDTH = 1000, HEIGHT = 700;

const int GRASS_NUM = 5;
const int GRASS_SIZE = 200;
const float MAX_GAUGE = 100.0f;
const float SUCCESS_TIME = 0.15f;
const float GAUGE_SPEED = 1.0f;

int grassX[GRASS_NUM];
int grassY[GRASS_NUM];
int grassType[GRASS_NUM];
int mouseX;
int mouseY;
int mouseInput;
int selectedGrass = -1;

float gauge = 0.0f;
float maxTime = 0.0f;

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

			// 既に置いた草と重なっていないか確認
			for (int j = 0; j < i; j++)
			{
				if (x < grassX[j] + GRASS_SIZE && x + GRASS_SIZE > grassX[j] && y < grassY[j] + GRASS_SIZE && y + GRASS_SIZE > grassY[j])
				{
					overlap = true;
					break;
				}
			}

			// 重なっていなければ位置を確定
			if (!overlap)
			{
				grassX[i] = x;
				grassY[i] = y;
				break;
			}
		}

		// 草の位置をランダムに決める
		int randType = GetRand(99);
		if (randType < 70) { grassType[i] = 0; }
		else if (randType < 95) { grassType[i] = 1; }
		else { grassType[i] = 2; }
	}

	while (1)
	{
		ClearDrawScreen();	// 画面をクリアする
		DrawGraph(0, 0, imgGround, false);

		GetMousePoint(&mouseX, &mouseY);
		mouseInput = GetMouseInput();

		if (mouseInput & MOUSE_INPUT_LEFT)
		{
			if (selectedGrass == -1)
			{
				for (int i = 0; i < GRASS_NUM; i++)
				{
					// クリックした場所が草かどうか
					if (mouseX >= grassX[i] && mouseX < grassX[i] + GRASS_SIZE && mouseY >= grassY[i] && mouseY < grassY[i] + GRASS_SIZE)
					{
						selectedGrass = i;
						break;
					}
				}
			}

			// 選択した草のゲージを増やす
			if (selectedGrass != -1)
			{
				if (gauge < MAX_GAUGE)
				{
					// MAXになるまでの時間を計算
					maxTime = GetNowCount() / 1000.0f + (MAX_GAUGE - gauge) / GAUGE_SPEED / 60.0f;
					gauge += GAUGE_SPEED;
					if (gauge >= MAX_GAUGE)
					{
						gauge = MAX_GAUGE;
					}
				}

			}
		}
		else
		{
			// マウスを離した
			if (selectedGrass != -1)
			{
				float releaseTime = GetNowCount() / 1000.0f;
				float timeDifference = releaseTime - maxTime;
				// MAXになる予定時間の前後0.15秒なら草を抜く
				if (gauge >= MAX_GAUGE && timeDifference >= -SUCCESS_TIME && timeDifference <= SUCCESS_TIME)
				{
					grassX[selectedGrass] = -1000;
					grassY[selectedGrass] = -1000;
				}

				// ゲージをリセット
				selectedGrass = -1;
				gauge = 0.0f;
				maxTime = 0.0f;
			}
		}

		for (int i = 0; i < GRASS_NUM; i++)
		{
			DrawGraph(grassX[i], grassY[i], imgGrass[grassType[i]], true);
		}

		if (selectedGrass != -1)
		{
			int gaugeWidth = 150;
			int gaugeHeight = 20;

			int gaugeX = grassX[selectedGrass] + (GRASS_SIZE - gaugeWidth) / 2;
			int gaugeY = grassY[selectedGrass] + GRASS_SIZE + 10;

			// ゲージの枠
			DrawBox(gaugeX, gaugeY, gaugeX + gaugeWidth, gaugeY + gaugeHeight, GetColor(255, 255, 255), false);

			// ゲージの中
			int fillWidth = static_cast<int>(gaugeWidth * (gauge / MAX_GAUGE));
			DrawBox(gaugeX, gaugeY, gaugeX + fillWidth, gaugeY + gaugeHeight, GetColor(255, 200, 0), true);
		}

		ScreenFlip();	// 裏画面の内容を表画面に反映させる
		WaitTimer(16);	// 一定時間待つ
		if (ProcessMessage() == -1) { break; }	// Windows から情報を受け取りエラーが起きたら終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) { break; }	// ESCキーが押されたら終了
	}
	DxLib_End();	// DXライブラリ使用の終了処理
	return 0;
}