#include "DxLib.h"
#include<stdlib.h>

const int WIDTH = 1000, HEIGHT = 700;
enum { TITLE, HELP, PLAY, RESULT };	// シーンを分けるための列挙定数

const int GRASS_NUM = 5;	// 画面に表示する草の数
const int GRASS_SIZE = 200;	// 草の画像サイズ
const float MAX_GAUGE = 100.0f;	// ゲージの最大値
const float SUCCESS_TIME = 0.1f;	// 成功判定の時間範囲（±0.1秒）
// 草ごとのゲージ速度
const float S_GRASS_SPEED = 2.0f;	// 小さい草
const float B_GRASS_SPEED = 1.4f;	// 大きい草
const float G_GRASS_SPEED = 0.8f;	// 金の草

int grassX[GRASS_NUM];	// 草のx座標
int grassY[GRASS_NUM];	// 草のy座標
int grassType[GRASS_NUM];	// 草の種類
int mouseX;
int mouseY;
int mouseInput;	// マウスが押されているかの確認
int selectedGrass = -1;	// 選択中の草の番号
int score = 0;
int time;	// 制限時間
int oldTime = 0;	// 1秒ごとの計測用
int scene = TITLE;
int resultTimer = 0;
int rankColor;
int alpha;	// 透明度を表す値
char rank;

float gauge = 0.0f;
float maxTime = 0.0f;	// ゲージがMAXになるまでの時間

bool overlap;	// 草同士が重なっていないか

int LoadGraphWithCheck(const char* file);

int LoadGraphWithCheck(const char* file)
{
	int res = LoadGraph(file);
	if (res == -1) { MessageBox(GetMainWindowHandle(), file, "画像読み込みに失敗", MB_OK | MB_ICONSTOP); }
	return res;
}

int LoadSoundMemWithCheck(const char* file)
{
	int res = LoadSoundMem(file);
	if (res == -1) { MessageBox(GetMainWindowHandle(), file, "音声読み込みに失敗", MB_OK | MB_ICONSTOP); }
	return res;
}

void DrawText(int x, int y, int col, const char* txt, int val, int siz)
{
	SetFontSize(siz);
	DrawFormatString(x + 2, y + 2, 0x000000, txt, val);
	DrawFormatString(x, y, col, txt, val);
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

	// 音声読み込み
	int sndTitle = LoadSoundMemWithCheck("sound/title.mp3");
	int sndPlay = LoadSoundMemWithCheck("sound/play.mp3");
	int seUnplug = LoadSoundMemWithCheck("sound/unplug.mp3");
	int sndResult = LoadSoundMemWithCheck("sound/result.mp3");

	// 草をランダムな位置に配置
	for (int i = 0; i < GRASS_NUM; i++)
	{
		while (1)
		{
			int x = GetRand(WIDTH - GRASS_SIZE);
			int y = GetRand(HEIGHT - GRASS_SIZE - 50);
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

	PlaySoundMem(sndTitle, DX_PLAYTYPE_LOOP);

	while (1)
	{
		ClearDrawScreen();	// 画面をクリアする

		switch (scene)
		{
		case TITLE:
			DrawGraph(0, 0, imgGround, false);
			DrawText(190, 100, 0x00a000, "草むしりマスター", 0, 80);
			DrawText(350, 250, 0x008000, "Sキー：ゲームスタート", 0, 30);
			DrawText(370, 300, 0x008000, "Hキー：操作説明", 0, 30);
			if (CheckHitKey(KEY_INPUT_S))
			{
				StopSoundMem(sndTitle);
				PlaySoundMem(sndPlay, DX_PLAYTYPE_LOOP);
				scene = PLAY;
				time = 60;
				score = 0;
				oldTime = GetNowCount();	// タイマー開始
				selectedGrass = -1;
				gauge = 0.0f;
				maxTime = 0.0f;
				resultTimer = 0;
			}
			else if (CheckHitKey(KEY_INPUT_H)) 
			{
				StopSoundMem(sndTitle);
				scene = HELP;
			}
			break;

		case HELP:
			DrawText(300, 30, 0xffffff, "ルール説明", 0, 60);
			DrawText(100, 150, 0xffffff, "1. 草をマウス左ボタンで長押し", 0, 30);
			DrawText(100, 200, 0xffffff, "2. ゲージがMAXになるタイミングで離す", 0, 30);
			DrawText(100, 250, 0xffffff, "3. タイミングが合えば草を抜ける", 0, 30);
			DrawText(100, 300, 0xffffff, "4. 失敗すると１点減点", 0, 30);

			DrawText(100, 340, 0xffffff, "--------------------------------------------------", 0, 30);

			DrawText(100, 380, 0xffffff, "小さい草 :  ＋2点", 0, 30);
			DrawText(100, 430, 0xffffff, "大きい草 :  ＋5点", 0, 30);
			DrawText(100, 480, 0xffffff, "金の草   : ＋10点", 0, 30);
			DrawText(100, 550, 0xffff00, "制限時間 : 60秒", 0, 30);

			DrawText(600, 380, 0xffffff, "Sランク → 120点以上", 0, 30);
			DrawText(600, 430, 0xffffff, "Aランク → 100点以上", 0, 30);
			DrawText(600, 480, 0xffffff, "Bランク → 80点以上", 0, 30);
			DrawText(600, 530, 0xffffff, "Cランク → 50点以上", 0, 30);
			DrawText(600, 580, 0xffffff, "Dランク → 50点未満", 0, 30);

			DrawText(400, 670, 0xffffff, "Sキー : ゲームスタート", 0, 20);
			if (CheckHitKey(KEY_INPUT_S))
			{
				PlaySoundMem(sndPlay, DX_PLAYTYPE_LOOP);
				scene = PLAY;
				time = 60;
				score = 0;
				oldTime = GetNowCount();	// タイマー開始
				selectedGrass = -1;
				gauge = 0.0f;
				maxTime = 0.0f;
				resultTimer = 0;
			}
			break;

		case PLAY:
			// 1秒経過したらタイマーを1減らす
			if (GetNowCount() - oldTime >= 1000)
			{
				time--;
				oldTime = GetNowCount();
			}

			if (time <= 0)
			{
				time = 0;
				StopSoundMem(sndPlay);
				PlaySoundMem(sndResult, DX_PLAYTYPE_LOOP);
				scene = RESULT;
				break;
			}

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
						// 草の種類に応じて速度を決める
						float gaugeSpeed = 0.0f;
						if (grassType[selectedGrass] == 0) { gaugeSpeed = S_GRASS_SPEED; }
						else if (grassType[selectedGrass] == 1) { gaugeSpeed = B_GRASS_SPEED; }
						else { gaugeSpeed = G_GRASS_SPEED; }

						// MAXになるまでの時間を計算
						maxTime = GetNowCount() / 1000.0f + (MAX_GAUGE - gauge) / gaugeSpeed / 60.0f;
						// ゲージを草ごとの速度で増やす
						gauge += gaugeSpeed;
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
					if (timeDifference >= -SUCCESS_TIME && timeDifference <= SUCCESS_TIME)
					{
						// 草の種類に応じてスコアを加算させる
						if (grassType[selectedGrass] == 0) { score += 2; }	// 小さい草
						else if (grassType[selectedGrass] == 1) { score += 5; }	// 大きい草
						else { score += 10; }	// 金の草

						// 抜いた草を新しいランダムな位置に生やす
						while (1)
						{
							int x = GetRand(WIDTH - GRASS_SIZE);
							int y = GetRand(HEIGHT - GRASS_SIZE - 50);
							overlap = false;

							// 他の草と重ならないか確認
							for (int i = 0; i < GRASS_NUM; i++)
							{
								if (i == selectedGrass) { continue; }
								if (x < grassX[i] + GRASS_SIZE && x + GRASS_SIZE > grassX[i] && y < grassY[i] + GRASS_SIZE && y + GRASS_SIZE > grassY[i])
								{
									overlap = true;
									break;
								}
							}

							// 重なっていなければ新しい位置にする
							if (!overlap)
							{
								grassX[selectedGrass] = x;
								grassY[selectedGrass] = y;
								break;
							}
						}

						// 新しく生える草の種類をランダムに決める
						int randType = GetRand(99);
						if (randType < 70) { grassType[selectedGrass] = 0; }
						else if (randType < 95) { grassType[selectedGrass] = 1; }
						else { grassType[selectedGrass] = 2; }
					}
					else { score -= 1; }	// ミスしたら１点減点

					// ゲージをリセット
					selectedGrass = -1;
					gauge = 0.0f;
					maxTime = 0.0f;
				}
			}

			for (int i = 0; i < GRASS_NUM; i++)
			{
				if (i == selectedGrass)
				{
					// 長押し中に草を伸ばす
					DrawExtendGraph(grassX[i], grassY[i] - 50, grassX[i] + GRASS_SIZE, grassY[i] + GRASS_SIZE, imgGrass[grassType[i]], true);
				}
				else { DrawGraph(grassX[i], grassY[i], imgGrass[grassType[i]], true); }
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
			DrawText(10, 10, 0xffffff, "score : %d", score, 30);
			DrawText(10, 50, 0xffffff, "time : %d", time, 30);
			break;

		case RESULT:
			resultTimer++;
			DrawGraph(0, 0, imgGround, false);
			if (resultTimer >= 30) { DrawText(325, 30, 0xffffff, "RESULT", 0, 120); }
			if (resultTimer >= 90) { DrawText(280, 300, 0xffffff, "スコア : %dpt", score, 60); }
			if (resultTimer >= 120) { DrawText(770, 270, 0xffffff, "ランク", 0, 40); }

			// スコアに応じてランクを決める
			if (score >= 120)
			{
				rank = 'S';
				rankColor = 0x0000ff;	// 青色
			}
			else if (score >= 100)
			{
				rank = 'A';
				rankColor = 0xffd700;	// 金
			}
			else if (score >= 80)
			{
				rank = 'B';
				rankColor = 0xc0c0c0;	// 銀
			}
			else if (score >= 50)
			{
				rank = 'C';
				rankColor = 0xcd7f32;	// 銅
			}
			else
			{
				rank = 'D';
				rankColor = 0x808080;	// 灰色
			}

			if (resultTimer >= 180)
			{
				// ランクを徐々に表示させる
				alpha = (resultTimer - 180) * 255 / 30;
				if (alpha > 255) { alpha = 255; }
				SetDrawBlendMode(DX_BLENDMODE_ALPHA, alpha);
				DrawText(800, 320, rankColor, "%c", rank, 150);
				SetDrawBlendMode(DX_BLENDMODE_NOBLEND, 0);
			}
			if (resultTimer >= 240)
			{
				DrawText(350, 600, 0xffffff, "Rキー：タイトルに戻る", 0, 30);
				DrawText(350, 650, 0xffffff, "Sキー：リスタート", 0, 30);
			}

			if (CheckHitKey(KEY_INPUT_R)) 
			{
				StopSoundMem(sndResult);
				PlaySoundMem(sndTitle, DX_PLAYTYPE_LOOP);
				scene = TITLE;
			}
			else if (CheckHitKey(KEY_INPUT_S))
			{
				StopSoundMem(sndResult);
				PlaySoundMem(sndPlay, DX_PLAYTYPE_LOOP);
				scene = PLAY;
				time = 60;
				score = 0;
				oldTime = GetNowCount();	// タイマー開始
				selectedGrass = -1;
				gauge = 0.0f;
				maxTime = 0.0f;
				resultTimer = 0;
			}
			break;
		}

		ScreenFlip();	// 裏画面の内容を表画面に反映させる
		WaitTimer(16);	// 一定時間待つ
		if (ProcessMessage() == -1) { break; }	// Windows から情報を受け取りエラーが起きたら終了
		if (CheckHitKey(KEY_INPUT_ESCAPE) == 1) { break; }	// ESCキーが押されたら終了
	}
	DxLib_End();	// DXライブラリ使用の終了処理
	return 0;
}