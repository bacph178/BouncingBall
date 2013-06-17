//
//  HelloWorldScene.cpp
//  Bouncing Ball
//
//  Created by macbook_016 on 2013/06/13.
//  Copyright __MyCompanyName__ 2013年. All rights reserved.
//
#include "HelloWorldScene.h"
#include "SimpleAudioEngine.h"

using namespace cocos2d;
using namespace CocosDenshion;

#define PTM_RATIO 32

enum {
    kTagParentNode = 1,
};

PhysicsSprite::PhysicsSprite()
: m_pBody(NULL)
{

}

void PhysicsSprite::setPhysicsBody(b2Body * body)
{
    m_pBody = body;
}

// this method will only get called if the sprite is batched.
// return YES if the physics values (angles, position ) changed
// If you return NO, then nodeToParentTransform won't be called.
bool PhysicsSprite::isDirty(void)
{
    return true;
}

// returns the transform matrix according the Chipmunk Body values
CCAffineTransform PhysicsSprite::nodeToParentTransform(void)
{
    b2Vec2 pos  = m_pBody->GetPosition();

    float x = pos.x * PTM_RATIO;
    float y = pos.y * PTM_RATIO;

    if ( isIgnoreAnchorPointForPosition() ) {
        x += m_obAnchorPointInPoints.x;
        y += m_obAnchorPointInPoints.y;
    }

    // Make matrix
    float radians = m_pBody->GetAngle();
    float c = cosf(radians);
    float s = sinf(radians);

    if( ! m_obAnchorPointInPoints.equals(CCPointZero) ){
        x += c*-m_obAnchorPointInPoints.x + -s*-m_obAnchorPointInPoints.y;
        y += s*-m_obAnchorPointInPoints.x + c*-m_obAnchorPointInPoints.y;
    }

    // Rot, Translate Matrix
    m_sTransform = CCAffineTransformMake( c,  s,
        -s,    c,
        x,    y );

    return m_sTransform;
}

HelloWorld::HelloWorld()
{
    setTouchEnabled( true );
    setAccelerometerEnabled( true );

    CCSize s = CCDirector::sharedDirector()->getWinSize();
    // init physics
    this->initPhysics();

//    CCSpriteBatchNode *parent = CCSpriteBatchNode::create("blocks.png", 100);
//    m_pSpriteTexture = parent->getTexture();
//
//    addChild(parent, 0, kTagParentNode);


  //  addNewSpriteAtPosition(ccp(s.width/2, s.height/2));

    CCLabelTTF *label = CCLabelTTF::create("DEMO BOX2D", "Marker Felt", 32);
    addChild(label, 0);
    label->setColor(ccc3(0,0,255));
    label->setPosition(ccp( s.width/2, s.height-50));
    
    scheduleUpdate();
}

HelloWorld::~HelloWorld()
{
    delete _world;
    _world = NULL;
    
    //delete m_debugDraw;
}

void HelloWorld::initPhysics()
{

    CCSize s = CCDirector::sharedDirector()->getWinSize();

    // Create a world
    b2Vec2 gravity = b2Vec2(0.0f, 0.0f);
    _world = new b2World(gravity);
    CCSprite *ground = CCSprite::create("court.png");
    ground->setPosition(ccp(s.width/2 ,s.height/2));
  //  CCLOG("%f, %f", ground->getPositionX(), ground->getPositionY());
    ground->setTag(1);
    this->addChild(ground,-1);
    
    // Create edges around the entire screen
    b2BodyDef groundBodyDef;
    groundBodyDef.position.Set(0,0);
    _groundBody = _world->CreateBody(&groundBodyDef);
    
    b2EdgeShape groundBox;
    b2FixtureDef groundBoxDef;
    groundBoxDef.shape = &groundBox;
    
    groundBox.Set(b2Vec2(0,0), b2Vec2(s.width/PTM_RATIO, 0));
    _bottomFixture = _groundBody->CreateFixture(&groundBoxDef);
    
    groundBox.Set(b2Vec2(0,0), b2Vec2(0, s.height/PTM_RATIO));
    _groundBody->CreateFixture(&groundBoxDef);
    
    groundBox.Set(b2Vec2(0, s.height/PTM_RATIO), b2Vec2(s.width/PTM_RATIO,
                                                              s.height/PTM_RATIO));
    _groundBody->CreateFixture(&groundBoxDef);
    
    groundBox.Set(b2Vec2(s.width/PTM_RATIO, s.height/PTM_RATIO),
                  b2Vec2(s.width/PTM_RATIO, 0));
    _groundBody->CreateFixture(&groundBoxDef);
    
    // Create sprite and add it to the layer
    CCSprite *ball = CCSprite::create("puck.png");
    ball->setPosition(ccp(s.width/2, s.height/2));
    ball->setTag(1);
    this->addChild(ball);
    
    // Create ball body
    b2BodyDef ballBodyDef;
    ballBodyDef.type = b2_dynamicBody;
    ballBodyDef.position.Set(100/PTM_RATIO, 100/PTM_RATIO);
    ballBodyDef.userData = ball;
    b2Body * ballBody = _world->CreateBody(&ballBodyDef);
    
    // Create circle shape
    b2CircleShape circle;
    circle.m_radius = 26.0/PTM_RATIO;
    
    // Create shape definition and add to body
    b2FixtureDef ballShapeDef;
    ballShapeDef.shape = &circle;
    ballShapeDef.density = 1.0f;
    ballShapeDef.friction = 0.f;
    ballShapeDef.restitution = 1.0f;
    _ballFixture = ballBody->CreateFixture(&ballShapeDef);
    
    b2Vec2 force = b2Vec2(10, 10);
    ballBody->ApplyLinearImpulse(force, ballBodyDef.position);
    
    // Create paddle and add it to the layer
    CCSprite *paddle1 = CCSprite::create("mallet.png");
    paddle1->setPosition(ccp(s.width, s.height/2));
    this->addChild(paddle1);

    // Create paddle body
    b2BodyDef paddleBodyDef;
    paddleBodyDef.type = b2_dynamicBody;
    paddleBodyDef.position.Set(s.width/PTM_RATIO, s.height/2/PTM_RATIO);
    paddleBodyDef.userData = paddle1;
    _paddleBody1 = _world->CreateBody(&paddleBodyDef);
    
    // Create paddle shape
    b2PolygonShape paddleShape1;
    paddleShape1.SetAsBox(paddle1->getContentSize().width/PTM_RATIO/2,
                         paddle1->getContentSize().height/PTM_RATIO/2);
    
    // Create shape definition and add to body
    b2FixtureDef paddleShapeDef1;
    paddleShapeDef1.shape = &paddleShape1;
    paddleShapeDef1.density = 10.0f;
    paddleShapeDef1.friction = 0.4f;
    paddleShapeDef1.restitution = 0.1f;
    _paddleFixture1 = _paddleBody1->CreateFixture(&paddleShapeDef1);
    
    // Create paddle and add it to the layer
    CCSprite *paddle2 = CCSprite::create("mallet.png");
    paddle1->setPosition(ccp(0, s.height/2));
    this->addChild(paddle2);
    
    // Create paddle body
    b2BodyDef paddleBodyDef2;
    paddleBodyDef2.type = b2_dynamicBody;
    paddleBodyDef2.position.Set(0/PTM_RATIO, s.height/2/PTM_RATIO);
    paddleBodyDef2.userData = paddle2;
    _paddleBody2 = _world->CreateBody(&paddleBodyDef2);
    
    // Create paddle shape
    b2PolygonShape paddleShape2;
    paddleShape2.SetAsBox(paddle2->getContentSize().width/PTM_RATIO/2,
                          paddle2->getContentSize().height/PTM_RATIO/2);
    
    // Create shape definition and add to body
    b2FixtureDef paddleShapeDef2;
    paddleShapeDef2.shape = &paddleShape2;
    paddleShapeDef2.density = 10.0f;
    paddleShapeDef2.friction = 0.4f;
    paddleShapeDef2.restitution = 0.1f;
    _paddleFixture2 = _paddleBody2->CreateFixture(&paddleShapeDef2);
    
    this->setTouchEnabled(true);
    
    this->schedule(schedule_selector(HelloWorld::update),1/60);
}

void HelloWorld::ccTouchesBegan(cocos2d::CCSet* touches, cocos2d::CCEvent *event)
{
//    if (_mouseJoint != NULL) return;
//    
//    CCTouch *myTouch = (CCTouch*)touches->anyObject();
//    CCPoint location = myTouch->getLocationInView();
//    location = CCDirector::sharedDirector()->convertToGL(location);
//    b2Vec2 locationWorld = b2Vec2(location.x/PTM_RATIO, location.y/PTM_RATIO);
//    
//    if (_paddleFixture1->TestPoint(locationWorld)) {
//        b2MouseJointDef md;
//        md.bodyA = _groundBody;
//        md.bodyB = _paddleBody;
//        md.target = locationWorld;
//        md.collideConnected = true;
//        md.maxForce = 1000.0f * _paddleBody->GetMass();
//        
//        _mouseJoint = (b2MouseJoint *)_world->CreateJoint(&md);
//        _paddleBody->SetAwake(true);
//    }
    CCSetIterator i;
    
    for (i = touches->begin(); i != touches->end(); i++) {
        if (_mouseJoint != NULL) return;
        CCTouch *touch = (CCTouch*)(*i);
        CCPoint tap = touch->getLocation();
        
        b2Vec2 locationWorld = b2Vec2(tap.x / PTM_RATIO, tap.y / PTM_RATIO);
        
        if (_paddleFixture1->TestPoint(locationWorld)) {
            b2MouseJointDef md;
            md.bodyA = _groundBody;
            md.bodyB = _paddleBody1;
            md.target = locationWorld;
            md.collideConnected = true;
            md.maxForce = 1000.0f * _paddleBody1->GetMass();
            
            _mouseJoint = (b2MouseJoint *)_world->CreateJoint(&md);
            _paddleBody1->SetAwake(true);
        }
        if (_paddleFixture2->TestPoint(locationWorld)) {
            b2MouseJointDef md;
            md.bodyA = _groundBody;
            md.bodyB = _paddleBody2;
            md.target = locationWorld;
            md.collideConnected = true;
            md.maxForce = 1000.0f * _paddleBody2->GetMass();
            
            _mouseJoint = (b2MouseJoint *)_world->CreateJoint(&md);
            _paddleBody2->SetAwake(true);
        }
    }

}

void HelloWorld::ccTouchesMoved(cocos2d::CCSet *touches, cocos2d::CCEvent *event)
{
    if (_mouseJoint == NULL) return;
    
    CCTouch *myTouch = (CCTouch *)touches->anyObject();
    CCPoint location = myTouch->getLocationInView();
    location = CCDirector::sharedDirector()->convertToGL(location);
    b2Vec2 locationWorld = b2Vec2(location.x/PTM_RATIO, location.y/PTM_RATIO);
    
    _mouseJoint->SetTarget(locationWorld);
}

void HelloWorld::ccTouchesCancelled(cocos2d::CCSet *touches, cocos2d::CCEvent *event) {
    
    if (_mouseJoint) {
        _world->DestroyJoint(_mouseJoint);
        _mouseJoint = NULL;
    }
    
}

void HelloWorld::ccTouchesEnded(cocos2d::CCSet *touches, cocos2d::CCEvent *event) {
    if (_mouseJoint) {
        _world->DestroyJoint(_mouseJoint);
        _mouseJoint = NULL;
    }
}

void HelloWorld::draw()
{
    //
    // IMPORTANT:
    // This is only for debug purposes
    // It is recommend to disable it
    //
    CCLayer::draw();

    ccGLEnableVertexAttribs( kCCVertexAttribFlag_Position );

    kmGLPushMatrix();

    _world->DrawDebugData();

    kmGLPopMatrix();
}


void HelloWorld::update(float dt)
{
    _world->Step(dt, 10, 10);
    for(b2Body *b = _world->GetBodyList(); b; b=b->GetNext()) {
        if (b->GetUserData() != NULL) {
            CCSprite *sprite = (CCSprite *)b->GetUserData();
            if (sprite->getTag() == 1) {
                static int maxSpeed = 10;
                
                b2Vec2 velocity = b->GetLinearVelocity();
                float32 speed = velocity.Length();
                
                if (speed > maxSpeed) {
                    b->SetLinearDamping(0.5);
                } else if (speed < maxSpeed) {
                    b->SetLinearDamping(0.0);
                }
                
            }
            sprite->setPosition(ccp(b->GetPosition().x * PTM_RATIO,b->GetPosition().y * PTM_RATIO));
           // sprite->setRotation(-1 * CC_RADIANS_TO_DEGREES(b->GetAngle()));
        }
    }
}

void HelloWorld::accelerometer(cocos2d::CCAcceleration *accelerometer,  cocos2d::CCAcceleration *acceleration)
{
    // Landscape left values
    b2Vec2 gravity(acceleration->y * 30, -acceleration->x * 30);
    _world->SetGravity(gravity);
}

CCScene* HelloWorld::scene()
{
    // 'scene' is an autorelease object
    CCScene *scene = CCScene::create();
    
    // add layer as a child to scene
    CCLayer* layer = new HelloWorld();
    scene->addChild(layer);
    layer->release();
    
    return scene;
}
