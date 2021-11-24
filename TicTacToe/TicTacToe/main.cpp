#include <SFML/Graphics.hpp>
#include <iostream>
#include <stdio.h>
#include <stdio.h>
#include <memory>
#include <map>
#include <string>
#include <vector>
#include <array>
#include <random>
#include <stdexcept>
#include <cassert>

namespace
{
	const sf::Vector2i WINDOW_SIZE(640, 480);
	const unsigned NUMBER_OF_PLAYERS = 3;
	const float SIZE = 70.f;
	const float UIPos = 0.2f;
	const unsigned DEPTH = 7;
	int DIM = 5;
	sf::Vector2f START_POINT(WINDOW_SIZE.x * 0.5f - DIM * SIZE * 0.5f, WINDOW_SIZE.y * 0.5f - DIM * SIZE * 0.5f);
	int modes = 0;
}

enum struct Player : unsigned
{
	None,
	User1,
	User2,
	Computer
};

template <typename Resource> static
void centerOrigin(Resource& resource)
{
	sf::FloatRect bounds = resource.getLocalBounds();
	resource.setOrigin(std::floor(bounds.left + bounds.width / 2.f), std::floor(bounds.top + bounds.height / 2.f));
}

class Tile : public sf::RectangleShape, private sf::NonCopyable
{

public:
	Tile() = default;

	void setOwner(Player player)
	{
		mOwner = player;
	}

	Player getOwner() const
	{
		return mOwner;
	}

private:
	Player mOwner = Player::None;
};

class World
{
	struct Move
	{
		unsigned x = 0;
		unsigned y = 0;
	};

public:
	explicit World(sf::RenderTarget& outputTarget);

	void Inititalize();

	bool isFull() const;
	bool isWinner(Player player) const;
	bool applyMove(Player player, sf::Uint32 row, sf::Uint32 column) const;
	bool applyAl(Player player) const;
	void draw();

private:
	Move minimax() const;
	int minSearch(int level, int alpha, int beta) const;
	int maxSearch(int level, int alpha, int beta) const;

private:
	sf::RenderTarget& mTarget;
	mutable unsigned mRemain;
	mutable std::array<Tile, 25> mTiles;
};

World::World(sf::RenderTarget& outputTarget)
	: mTarget(outputTarget)
	, mTiles()
{
	
}

void World::Inititalize()
{
	mRemain = DIM * DIM;
	START_POINT.x = WINDOW_SIZE.x * 0.5f - DIM * SIZE * 0.5f;
	START_POINT.y = WINDOW_SIZE.y * 0.5f - DIM * SIZE * 0.5f;

	sf::Vector2f startPosition(START_POINT);


	for (unsigned i = 0; i < DIM; ++i)
	{
		for (unsigned j = 0; j < DIM; ++j)
		{
			unsigned index = j * DIM + i;

			mTiles[index].setSize(sf::Vector2f(SIZE, SIZE));
			mTiles[index].setPosition(startPosition);
			mTiles[index].setOutlineThickness(2.f);
			mTiles[index].setFillColor(sf::Color::Black);
			mTiles[index].setOutlineColor(sf::Color::White);

			startPosition.x += SIZE;
		}

		startPosition.y += SIZE;
		startPosition.x = START_POINT.x;
	}
}

void World::draw()
{
	for (int i = 0; i < DIM*DIM; i++)
	{
		mTarget.draw(mTiles[i]);
	}
}

bool World::applyMove(Player player, sf::Uint32 row, sf::Uint32 column) const
{
	unsigned index = row + DIM * column;

	if ((index > mTiles.size()) || (mTiles[index].getOwner() != Player::None) || row >= DIM || column >= DIM)
	{
		return false;
	}

	--mRemain;

	mTiles[index].setOwner(player);

	switch (player)
	{
	case Player::User1:
		mTiles[index].setFillColor(sf::Color::Blue);
		break;
	case Player::Computer:
		mTiles[index].setFillColor(sf::Color::Red);
		break;
	case Player::User2:
		mTiles[index].setFillColor(sf::Color::Red);
		break;
	}

	return true;
}

bool World::isFull() const
{
	return (mRemain == 0);
}

bool World::applyAl(Player player) const
{
	Move move = minimax();
	return applyMove(player, move.x, move.y);
}

World::Move World::minimax() const
{
	static int level = 0;
	int score = std::numeric_limits<int>::max();
	Move move;

	for (unsigned i = 0; i < DIM; i++)
	{
		for (unsigned j = 0; j < DIM; j++)
		{
			unsigned index = j * DIM + i;
				if (mTiles[index].getOwner() == Player::None)
				{
					mTiles[index].setOwner(Player::Computer);
					--mRemain;

					int temp = maxSearch(level, std::numeric_limits<int>::min(), std::numeric_limits<int>::max());

					if (temp < score)
					{
						score = temp;
						move.x = i;
						move.y = j;
					}

					mTiles[index].setOwner(Player::None);
					++mRemain;
				}
		}
	}

	return move;
}

int World::maxSearch(int level, int alpha, int beta) const
{
	if (isWinner(Player::User1)) { return 10; }
	else if (isWinner(Player::Computer)) { return -10; }
	else if (isFull()) { return 0; }

	int score = std::numeric_limits<int>::min();

	if (level >= DEPTH)
	{
		return score;
	}

	for (unsigned i = 0; i < DIM; i++)
	{
		for (unsigned j = 0; j < DIM; j++)
		{
			unsigned index = j * DIM + i;
			if (mTiles[index].getOwner() == Player::None)
			{
				mTiles[index].setOwner(Player::User1);
				--mRemain;

				score = std::max(score, minSearch(level + 1, alpha, beta) - level);

				mTiles[index].setOwner(Player::None);
				++mRemain;

				if (beta <= alpha) return alpha;
			}
		}
	}

	return score;
}

int World::minSearch(int level, int alpha, int beta) const
{
	if (isWinner(Player::User1)) { return 10; }
	else if (isWinner(Player::Computer)) { return -10; }
	else if (isFull()) { return 0; }

	int score = std::numeric_limits<int>::max();

	if (level >= DEPTH)
	{
		return score;
	}

	for (unsigned i = 0; i < DIM; i++)
	{
		for (unsigned j = 0; j < DIM; j++)
		{
			unsigned index = j * DIM + i;

			if (mTiles[index].getOwner() == Player::None)
			{
				mTiles[index].setOwner(Player::Computer);
				--mRemain;

				score = std::min(score, maxSearch(level + 1, alpha, beta) + level);

				mTiles[index].setOwner(Player::None);
				++mRemain;

				if (beta <= alpha) return beta;
			}
		}
	}

	return score;
}

bool World::isWinner(Player player) const
{
	// check for row or column wins
	for (unsigned i = 0; i < DIM; ++i)
	{
		bool rowwin = true;
		bool colwin = true;
		for (unsigned j = 0; j < DIM; ++j)
		{
			rowwin &= mTiles[i*DIM + j].getOwner() == player;
			colwin &= mTiles[j*DIM + i].getOwner() == player;
		}
		if (colwin || rowwin)
			return true;
	}

	// check for diagonal wins
	bool diagwin = true;
	for (unsigned i = 0; i < DIM; ++i)
		diagwin &= mTiles[i*DIM + i].getOwner() == player;

	if (diagwin)
		return true;

	diagwin = true;
	for (unsigned i = 0; i < DIM; ++i)
		diagwin &= mTiles[i*DIM + (DIM - i - 1)].getOwner() == player;

	return diagwin;
}

class Game : private sf::NonCopyable
{
public:
	Game();
	void run();

	bool next1 = false;
	bool next2 = false;
	bool next3 = false;

private:

	void processEvents();
	void modeChoose();
	void modeInput();
	void gridChoose();
	void gridInput();
	void displayName();
	void enterName(sf::String &p, sf::Text &player);
	void load();
	void update();
	void render();

	bool CheckStateOne(sf::Vector2f pos);
	bool CheckStateTwo(sf::Vector2f pos);

	sf::RenderWindow mWindow;

	sf::Font mFont;
	sf::Text mText;
	sf::Text mTitle;
	sf::Text pvp, pvc;
	sf::Text grid[3];
	sf::String p1, p2;
	sf::Text player1, player2;
	sf::Image bar_i;
	sf::Texture bar_t;
	sf::Sprite bar[5];

	sf::Image user1_i, user2_i;
	sf::Texture user1_t, user2_t;
	sf::Sprite user1, user2;

	World mWorld;
	std::array<Player, NUMBER_OF_PLAYERS> mPlayers;
};

Game::Game()
	: mWindow(sf::VideoMode(WINDOW_SIZE.x, WINDOW_SIZE.y), "Tic Tac Toe - SFML")
	, mFont()
	, mText()
	, mTitle()
	, mWorld(mWindow)
	, mPlayers({ { Player::User1, Player::Computer, Player::User2} })
{
	mWindow.setVerticalSyncEnabled(true);

	load();
}

void Game::run()
{
	while (mWindow.isOpen())
	{
		processEvents();
		if (!next1 && !next2 && !next3)
		{
			mText.setString("Choose The Mode ");
			modeChoose();
			modeInput();
		}
		else if (next1 && !next2 && !next3)
		{
			mText.setString("Enter Your Name");
			displayName();
			enterName(p1, player1);
			if (modes == 2)
			{
				p2 = "AI";
				player2.setString("AI");
			}
			else
			{
				enterName(p2, player2);
			}
			next2 = true;
		}
		else if (next1 && next2 && !next3)
		{
			mText.setString("Choose The Grid Size ");
			gridChoose();
			gridInput();
		}
		else if (next3)
		{
			processEvents();
			update();
			render();
		}
	}
}

void Game::enterName(sf::String &p, sf::Text &player)
{
	sf::Event event;
	while (mWindow.isOpen())
	{
		while (mWindow.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				mWindow.close();
			case  sf::Event::TextEntered:
			{
				if (event.text.unicode < 128)
				{
					p += event.text.unicode;
					player.setString(p);
					mWindow.draw(player);
					mWindow.display();
				}
			}
			case sf::Event::KeyPressed:
				if (event.key.code == sf::Keyboard::Enter)
				{
					return;
				}
			}
		}
	}
}

void Game::modeInput()
{
	sf::Event event;
	while (mWindow.isOpen())
	{
		while (mWindow.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				mWindow.close();
			case sf::Event::MouseButtonPressed:
			{
				if (event.mouseButton.button == sf::Mouse::Button::Left)
					if (CheckStateOne(mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow))))
					{
						return;
					}
			}
			}
		}
	}
}

void Game::gridInput()
{
	sf::Event event;
	while (mWindow.isOpen())
	{
		while (mWindow.pollEvent(event))
		{
			switch (event.type)
			{
			case sf::Event::Closed:
				mWindow.close();
			case sf::Event::MouseButtonPressed:
			{
				if (event.mouseButton.button == sf::Mouse::Button::Left)
					if (CheckStateTwo(mWindow.mapPixelToCoords(sf::Mouse::getPosition(mWindow))))
					{
						return;
					}
			}
			}
		}
	}
}

bool Game::CheckStateOne(sf::Vector2f pos)
{
	if (bar[0].getGlobalBounds().contains(pos))
	{
		modes = 1;
		next1 = true;
		return true;
	}
	if (bar[1].getGlobalBounds().contains(pos))
	{
		modes = 2;
		next1 = true;
		return true;
	}
	return false;
}

bool Game::CheckStateTwo(sf::Vector2f pos)
{
	if (bar[2].getGlobalBounds().contains(pos))
	{
		DIM = 3;
		next3 = true;
		mText.setString("Game Started");
		mWorld.Inititalize();
		return true;
	}
	if (bar[3].getGlobalBounds().contains(pos))
	{
		DIM = 4;
		next3 = true;
		mText.setString("Game Started");
		mWorld.Inititalize();
		return true;
	}
	if (bar[4].getGlobalBounds().contains(pos))
	{
		DIM = 5;
		next3 = true;
		mText.setString("Game Started");
		mWorld.Inititalize();
		return true;
	}
	return false;
}

void Game::modeChoose()
{
	mWindow.clear();
	mWindow.draw(bar[0]);
	mWindow.draw(bar[1]);
	mWindow.draw(user1);
	mWindow.draw(user2);
	mWindow.draw(pvp);
	mWindow.draw(pvc);
	mWindow.draw(mTitle);
	mWindow.draw(mText);
	mWindow.display();
}

void Game::gridChoose()
{
	mWindow.clear();
	mWindow.draw(player1);
	mWindow.draw(player2);
	mWindow.draw(user1);
	mWindow.draw(user2);
	mWindow.draw(bar[2]);
	mWindow.draw(bar[3]);
	mWindow.draw(bar[4]);
	mWindow.draw(grid[0]);
	mWindow.draw(grid[1]);
	mWindow.draw(grid[2]);
	mWindow.draw(mTitle);
	mWindow.draw(mText);
	mWindow.display();
}

void Game::displayName()
{
	mWindow.clear();
	mWindow.draw(player1);
	mWindow.draw(player2);
	mWindow.draw(bar[0]);
	mWindow.draw(bar[1]);
	mWindow.draw(user1);
	mWindow.draw(user2);
	mWindow.draw(pvp);
	mWindow.draw(pvc);
	mWindow.draw(mTitle);
	mWindow.draw(mText);
	mWindow.display();
}

void Game::load()
{
	if (!mFont.loadFromFile("Resource\\Sansation.ttf"))
		throw std::runtime_error("Failed to load font");

	if (!this->user1_i.loadFromFile("Resource\\User1.png"))
		throw std::runtime_error("Failed to sprite");
	if (!this->user1_t.loadFromImage(this->user1_i))
		throw std::runtime_error("Failed to sprite");
	user1.setTexture(user1_t);
	user1.setPosition(mWindow.getSize().x * 0.5f - 50.f, 100.f);

	if (!this->user2_i.loadFromFile("Resource\\User2.png"))
		throw std::runtime_error("Failed to sprite");
	if (!this->user2_t.loadFromImage(this->user2_i))
		throw std::runtime_error("Failed to sprite");
	user2.setTexture(user2_t);
	user2.setPosition(mWindow.getSize().x * 0.5f + 100.f, 100.f);

	if (!this->bar_i.loadFromFile("Resource\\Bar.png"))
		throw std::runtime_error("Failed to sprite");
	if (!this->bar_t.loadFromImage(this->bar_i))
		throw std::runtime_error("Failed to sprite");
	bar[0].setTexture(bar_t);
	bar[0].setPosition(mWindow.getSize().x * UIPos - 60.f, 150.f);
	bar[1].setTexture(bar_t);
	bar[1].setPosition(mWindow.getSize().x * UIPos - 60.f, 250.f);

	bar[2].setTexture(bar_t);
	bar[2].setPosition(mWindow.getSize().x * UIPos - 60.f, 150.f);
	bar[3].setTexture(bar_t);
	bar[3].setPosition(mWindow.getSize().x * UIPos - 60.f, 250.f);
	bar[4].setTexture(bar_t);
	bar[4].setPosition(mWindow.getSize().x * UIPos - 60.f, 350.f);

	player1.setFont(mFont);
	player1.setCharacterSize(15);
	player1.setFillColor(sf::Color::White);
	player1.setPosition(mWindow.getSize().x * 0.5f, 250.f);

	player2.setFont(mFont);
	player2.setCharacterSize(15);
	player2.setFillColor(sf::Color::White);
	player2.setPosition(mWindow.getSize().x * 0.5f + 150, 250.f);

	pvp.setString("PVP");
	pvp.setFont(mFont);
	pvp.setCharacterSize(15);
	pvp.setFillColor(sf::Color::White);
	pvp.setPosition(mWindow.getSize().x * UIPos - 10.f, 160.f);

	pvc.setString("PVC");
	pvc.setFont(mFont);
	pvc.setCharacterSize(15);
	pvc.setFillColor(sf::Color::White);
	pvc.setPosition(mWindow.getSize().x * UIPos - 10.f, 260.f);

	grid[0].setString("3X3");
	grid[0].setFont(mFont);
	grid[0].setCharacterSize(15);
	grid[0].setFillColor(sf::Color::White);
	grid[0].setPosition(mWindow.getSize().x * UIPos - 10.f, 160.f);

	grid[1].setString("4X4");
	grid[1].setFont(mFont);
	grid[1].setCharacterSize(15);
	grid[1].setFillColor(sf::Color::White);
	grid[1].setPosition(mWindow.getSize().x * UIPos - 10.f, 260.f);

	grid[2].setString("5X5");
	grid[2].setFont(mFont);
	grid[2].setCharacterSize(15);
	grid[2].setFillColor(sf::Color::White);
	grid[2].setPosition(mWindow.getSize().x * UIPos - 10.f, 360.f);


	mText.setFont(mFont);
	mText.setStyle(sf::Text::Bold);
	mText.setCharacterSize(20);
	mText.setFillColor(sf::Color::White);
	mText.setPosition(30.f, mWindow.getSize().y - 50.f);
	centerOrigin(mText);

	mTitle.setString("Colourful Tic Tac Toe");
	mTitle.setFont(mFont);
	mTitle.setStyle(sf::Text::Bold);
	mTitle.setCharacterSize(30);
	mTitle.setFillColor(sf::Color::White);
	mTitle.setPosition(mWindow.getSize().x * 0.5f, 50.f);
	centerOrigin(mTitle);
}

void Game::processEvents()
{
	sf::Event event;
	while (mWindow.pollEvent(event))
	{
		if (event.type == sf::Event::Closed)
			mWindow.close();
		if (event.type == sf::Event::MouseButtonPressed)
			return;
	}
}

void Game::update()
{
	static bool winner = false;
	static bool tie = false;
	static unsigned index = 0;

	if (tie || winner) return;

	switch (mPlayers[index])
	{
	case Player::User1:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			const sf::Vector2i& position = sf::Mouse::getPosition(mWindow);

			if (position.x > START_POINT.x &&
				position.y > START_POINT.y &&
				position.x < (START_POINT.x + (DIM*SIZE)) &&
				position.y < (START_POINT.y + (DIM*SIZE)))
			{
				unsigned row = static_cast<unsigned>((position.y - START_POINT.y) / SIZE);
				unsigned col = static_cast<unsigned>((position.x - START_POINT.x) / SIZE);

				if (mWorld.applyMove(Player::User1, row, col)) {
					winner = mWorld.isWinner(Player::User1);
					if (!winner) {
						if (modes == 1)
						{
							index = 2;
						}
						else
						{
							index = 1;
						}
					}
				}
			}
		}
		break;
	case Player::Computer:
		if (mWorld.applyAl(Player::Computer)) {
			winner = mWorld.isWinner(Player::Computer);
			if (!winner) {
				index = 0;
			}
		}
		break;
	case Player::User2:
		if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
		{
			const sf::Vector2i& position = sf::Mouse::getPosition(mWindow);

			if (position.x > START_POINT.x &&
				position.y > START_POINT.y &&
				position.x < (START_POINT.x + (DIM*SIZE)) &&
				position.y < (START_POINT.y + (DIM*SIZE)))
			{
				unsigned row = static_cast<unsigned>((position.y - START_POINT.y) / SIZE);
				unsigned col = static_cast<unsigned>((position.x - START_POINT.x) / SIZE);

				if (mWorld.applyMove(Player::User2, row, col)) {
					winner = mWorld.isWinner(Player::User2);
					if (!winner) {
						index = 0;
					}
				}
			}
		}
		break;
	}

	if (winner)
	{
		mText.setString("The Winner: " + std::string((mPlayers[index] == Player::User1) ? p1 : p2));
		return;
	}

	tie = mWorld.isFull();

	if (tie)
	{
		mText.setString("*** Tie ***");
		return;
	}
}

void Game::render()
{
	mWindow.clear();
	mWorld.draw();
	mWindow.draw(mTitle);
	mWindow.draw(mText);
	mWindow.display();
}

int main()
{
	try
	{
		Game game;
		game.run();
	}
	catch (std::runtime_error& e)
	{
		std::cout << "\nException: " << e.what() << std::endl;
		return 1;
	}
}