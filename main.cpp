#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <sstream>
/* I think I've included string only because that is currently how I plan
/* to display the lives and score counts, until I find out a better way.
*/
#include <string>

using namespace std;
using namespace sf;

//constexpr makes immutable constants for our window dimensions, for efficiency.
constexpr int windowWidth{800}, windowHeight{600};
constexpr float ballRadius{10.f}, ballVelocity{7.f};
constexpr float paddleWidth{60.f}, paddleHeight{20.f}, paddleVelocity{7.f};
constexpr float brickWidth{60.f}, brickHeight{20.f};
constexpr int countBricksX{11}, countBricksY{4};

int lives = 3;

/* I don't know why I've placed these parts of the sound element right here, though
/* I imagine it was to keep them at the top with all the other 'constant declarations'.
*/
sf::SoundBuffer buffer;
sf::Sound sound;
sf::Text text;
std::ostringstream convert;

struct Ball
{
    //Initializes the CircleShape.
    CircleShape shape;
    //Vector that stores a physics
    Vector2f velocity{-ballVelocity, -ballVelocity};

    float x() { return shape.getPosition().x; }
    float y() { return shape.getPosition().y; }
    float left() { return x() - shape.getRadius(); }
    float right() { return x() + shape.getRadius(); }
    float top() { return y() - shape.getRadius(); }
    float bottom() { return y() + shape.getRadius(); }

    //Ball constructor
    Ball(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setRadius(ballRadius);
        shape.setFillColor(Color::Green);
        shape.setOrigin(ballRadius, ballRadius);
    }

    //Updates the ball's location.
    void update() {
        shape.move(velocity);
        wallCollision();
        }

    void wallCollision()
    {
        if (left() <= 0.f)
            velocity.x = ballVelocity;
        if (right() >= 800.f)
            velocity.x = -ballVelocity;
        if (top() <= 0.f)
            velocity.y = ballVelocity;
        if (top() >= 600.f)
            velocity.y = -ballVelocity;
            lives -= 1;
            //if(lives==0)

    }
};

struct Paddle
{
    //Makes the paddle
    RectangleShape shape;
    Vector2f velocity;

    Paddle(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({paddleWidth, paddleHeight});
        shape.setFillColor(Color::Red);
        shape.setOrigin(paddleWidth/2.f, paddleHeight/2.f);
    }

    void update()
    {
        shape.move(velocity);

        if(Keyboard::isKeyPressed(Keyboard::Left) && left() > 0.f)
            velocity.x = -paddleVelocity;
        else if(Keyboard::isKeyPressed(Keyboard::Right) && right() < windowWidth)
            velocity.x = paddleVelocity;
        else
            velocity.x = 0;
    }

    float x() { return shape.getPosition().x; }
    float y() { return shape.getPosition().y; }
    float left() { return x() - shape.getSize().x/2.f; }
    float right() { return x() + shape.getSize().x/2.f; }
    float top() { return y() - shape.getSize().y/2.f; }
    float bottom() { return y() + shape.getSize().y/2.f; }

};

struct Brick
{
    RectangleShape shape;

    bool destroyed{false};

    Brick(float mX, float mY)
    {
        shape.setPosition(mX, mY);
        shape.setSize({brickWidth, brickHeight});
        shape.setFillColor(Color::Yellow);
        shape.setOrigin(brickWidth/2.f, brickHeight/2.f);
    }

    float x() { return shape.getPosition().x; }
    float y() { return shape.getPosition().y; }
    float left() { return x() - shape.getSize().x/2.f; }
    float right() { return x() + shape.getSize().x/2.f; }
    float top() { return y() - shape.getSize().y/2.f; }
    float bottom() { return y() + shape.getSize().y/2.f; }

};

template<class T1, class T2> bool isIntersecting(T1& mA, T2& mB)
{
    return mA.right() >= mB.left() && mA.left() <= mB.right()
           && mA.bottom() >= mB.top() && mA.top() <= mB.bottom();
}

void testCollision(Paddle& mPaddle, Ball& mBall)
{
    if (!isIntersecting(mPaddle, mBall)) return;

    mBall.velocity.y = -ballVelocity;
    sound.play();

    if (mBall.x() < mPaddle.x()) mBall.velocity.x = -ballVelocity;
    else mBall.velocity.x = ballVelocity;

}

void testCollision(Brick& mBrick, Ball& mBall)
{
    if(!isIntersecting(mBrick, mBall)) return;

    mBrick.destroyed = true;
    sound.play();

    float overlapLeft{mBrick.left() - mBall.right()};
    float overlapRight{mBall.left() - mBrick.right()};
    float overlapTop{mBrick.bottom() - mBall.top()};
    float overlapBottom{mBrick.bottom() - mBall.top()};

    bool ballFromLeft{abs(overlapLeft) < abs(overlapRight)};

    bool ballFromTop{abs(overlapTop) < abs(overlapBottom)};

    float minOverlapX{ballFromLeft ? overlapLeft : overlapRight};
    float minOverlapY{ballFromTop ? overlapTop : overlapBottom};

    if(abs(minOverlapX) < abs(minOverlapY))
        mBall.velocity.x = ballFromLeft ? -ballVelocity : ballVelocity;
    else
        mBall.velocity.y = ballFromTop ? -ballVelocity : ballVelocity;
}

int main()
{
    Ball ball(windowWidth/2, windowHeight/2);
    Paddle paddle(windowWidth/2, windowHeight-50);

    // We will use an `std::vector` to contain any number
	// of `Brick` instances.
	vector<Brick> bricks;

	// We fill up our vector via a 2D for loop, creating
	// bricks in a grid-like pattern.
	for(int iX{0}; iX < countBricksX; ++iX)
		for(int iY{0}; iY < countBricksY; ++iY)
			bricks.emplace_back((iX + 1) * (brickWidth + 3) + 22,
								(iY + 2) * (brickHeight + 3));

    //for(auto& Brick : bricks) cout << Brick.shape.getPosition().x << "\t" << Brick.shape.getPosition().y << endl;


    RenderWindow window({windowWidth, windowHeight}, "Hell Yeah!");
    window.setFramerateLimit(60);

    sf::Music music;
    if(!music.openFromFile("TheSmuggler.flac"))
        return EXIT_FAILURE;

    music.play();

    if(!buffer.loadFromFile("Blip_Select3.wav"))
        return -1;

    sound.setBuffer(buffer);

    convert << lives;
    string temp;
    temp = convert.str();
    text.setString(temp);
    text.setCharacterSize(16);
    text.setPosition(400.f, 300.f);
    text.setColor(Color::Yellow);

    //Game Loop
    while (window.isOpen())
    {
        window.clear(Color::Black);

        Event event;
        while (window.pollEvent(event))
        {
            if (event.type == Event::Closed) window.close();
            if (Keyboard::isKeyPressed(Keyboard::Escape)) window.close();
        }

        ball.update();
        paddle.update();
        testCollision(paddle, ball);

        for(auto& brick : bricks) testCollision(brick, ball);

        bricks.erase(remove_if(begin(bricks), end(bricks),
                               [](const Brick& mBrick){ return mBrick.destroyed; }),
                               end(bricks));

        window.draw(ball.shape);
        window.draw(paddle.shape);
        window.draw(text);
        for (auto& brick : bricks) window.draw(brick.shape);
        window.display();
    }

    return 0;
}
