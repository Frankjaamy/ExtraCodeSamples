#include "Hero.h"
#include "AnimationTool.h"
#include "ControllEvents.h"
#include "ControllReceiver.h"
#define CLASS_NAME Hero
/*
 In the beginning, Please pardon me for my inadequate English.
 
 This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.


 This is part of the inplementation of Hero class in the game Blade-Remake. Here I used a Finite State Machine class to define and manage
 all hero actions, states and behaviors.

 Please note that I did not write this Finite State Machine class. It belongs to YongWu Wang (http://www.contextfree.net/wangyw/ ). That's why
 I did not upload the code of this class. What I want to show is that I read his code thoroughly, and understand the algorithm and data structure and macros
 that run behind. That's why I am able to use and customize a Finite State Machine system in my own game.

 Also in this class, you can see there are different components -- Hero class needs not to know all the details, instead it has many modules and components 
 to do the work like animation component, combat skill component and collide component.
*/

BEGIN_STATE_TRANSITION(HeroState)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Alive)
    //ON_ENTRY(onStand);
    ON_ALL_EVENT_TRANSITION(isDead, die, Die)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Idle)
    ON_ENTRY(onStand);
    ON_EVENT_INTERNAL(EVENT_DEFAULT, Void, stand)
    ON_EVENT_TRANSITION(EVENT_WALK, isAlive, onWalk, Walk)
    ON_EVENT_TRANSITION(EVENT_RUN, isAlive, onRun, Run)
    ON_EVENT_TRANSITION(EVENT_CHARGE, isAlive, onCharge, Charge)
    ON_EVENT_TRANSITION(EVENT_JUMP, isAlive, onJump, Jump)
    ON_EVENT_TRANSITION(EVENT_ATTACK, isAlive, attack, Attack)
    ON_EVENT_TRANSITION(EVENT_HITTEN, isAlive, onHitten, Hitten)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Walk)
    ON_ENTRY(onRun);
    ON_EVENT_INTERNAL(EVENT_WALK, Void, moveGround)
    ON_EVENT_TRANSITION(EVENT_DEFAULT,stand, onStand, Idle)
    ON_EVENT_TRANSITION(EVENT_RUN, isAlive, onRun, Run)
    ON_EVENT_TRANSITION(EVENT_CHARGE, isAlive, onCharge, Charge)
    ON_EVENT_TRANSITION(EVENT_JUMP, isAlive, onJump, Jump)
    ON_EVENT_TRANSITION(EVENT_ATTACK, isAlive, attack, Attack)
    ON_EVENT_TRANSITION(EVENT_HITTEN, isAlive, onHitten, Hitten)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Run)
    ON_ENTRY(onRun);
    ON_EVENT_INTERNAL(EVENT_RUN, Void, moveGround)
    ON_EVENT_TRANSITION(EVENT_DEFAULT,Void, onWalk, Walk)
    ON_EVENT_TRANSITION(EVENT_CHARGE, isAlive, onCharge, Charge)
    ON_EVENT_TRANSITION(EVENT_JUMP, isAlive, onJump, Jump)
    ON_EVENT_TRANSITION(EVENT_ATTACK, isAlive, attack, Attack)
    ON_EVENT_TRANSITION(EVENT_HITTEN, isAlive, onHitten, Hitten)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Charge)
    ON_ENTRY(onCharge);
    ON_EVENT_INTERNAL(EVENT_CHARGE, Void, moveGround)
    ON_EVENT_TRANSITION(EVENT_DEFAULT,Void, onWalk, Walk)
    ON_EVENT_TRANSITION(EVENT_RUN, isAlive, onRun, Run)
    ON_EVENT_TRANSITION(EVENT_JUMP, isAlive, onJump, Jump)
    ON_EVENT_TRANSITION(EVENT_ATTACK, isAlive, attack, Attack)
    ON_EVENT_TRANSITION(EVENT_HITTEN, isAlive, onHitten, Hitten)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Jump)
    ON_ENTRY(onJump)
    ON_EVENT_TRANSITION(EVENT_LAND,Void,onStand,Idle)
    ON_ALL_EVENT_INTERNAL(isUpInAir, moveAir)
    ON_EVENT_TRANSITION(EVENT_HITTEN, isAlive, onHitten, Hitten)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Die)
END_STATE_TRANSITION



BEGIN_STATE_TRANSITION(Attack)
    ON_EVENT_INTERNAL(EVENT_ATTACK, isAlive, attack);
    ON_EVENT_TRANSITION(EVENT_ATTACK_OVER, isAlive, Void, Idle);
    ON_EVENT_TRANSITION(EVENT_HITTEN, isAlive, onHitten, Hitten)
END_STATE_TRANSITION

BEGIN_STATE_TRANSITION(Hitten)
    ON_EVENT_TRANSITION(EVENT_RECOVER, isAlive, Void, Idle);
END_STATE_TRANSITION

bool listenKeyDown = true;
Texture2D * texture;
double timer;

Hero::Hero(GameStage1* stage): Player(stage,TYPE_HERO)
{

}

Hero::~Hero(void)
{
}
void Hero::init(){
    
    Player::init();
    m_pCollide = new CollideIndicator();
    m_pCollide->setPlayer(this);
    MyCollideBox * mp = new MyCollideBox(0,this,0,0,20,30);
    setBoxWidth(mp->box.size.width);
    setBoxHeight(mp->box.size.height);
    m_pCollide->createBoxes(true,mp);
    m_sprite->setAnchorPoint(Point(0.5,0));
    
    //State Init
    INIT_ROOT_STATE(HeroState)
    INIT_STATE(HeroState, Alive)
        INIT_STATE(Alive, Idle)
        INIT_STATE(Alive, Walk)
        INIT_STATE(Alive, Run)
        INIT_STATE(Alive, Charge)
        INIT_STATE(Alive, Jump)
        INIT_STATE(Alive, Attack)
        INIT_STATE(Alive, Hitten)
    INIT_STATE(HeroState, Die)
    ActiveDefaultState();
}

bool Hero::isUpInAir(WORD wParam, DWORD lParam){
    return getCurPoint().y>m_pStageBelonged->getGroundHeight()+m_boxHeight;
}

bool Hero::stand(WORD wParam, DWORD lParam)
{
    return getHoriVelocity()==0;
}
bool Hero::onStand(WORD wParam, DWORD lParam)
{
    m_sprite->setSpriteFrame("Hero_Stand_1.png");
    m_sprite->setAnchorPoint(Point(0.5,0));
    
    AnimationTool::playAnimation(m_sprite,"Hero_Stand");
    m_vVelocity = 0.0f;
    m_curPoint.y=m_nextPoint.y=m_pStageBelonged->getGroundHeight()+m_boxHeight;
    
    m_bIsBusy = false;
    return true;
}

bool Hero::onWalk(WORD wParam, DWORD lParam)
{
    AnimationTool::playAnimation(m_sprite,"Hero_Walk");
    return true;
}

bool Hero::onRun(WORD wParam, DWORD lParam)
{
    AnimationTool::playAnimation(m_sprite,"Hero_Run");
    return true;
}

bool Hero::onJump(WORD wParam, DWORD lParam)
{
    AnimationTool::playAnimation(m_sprite,"Hero_Jump");
    
    m_vVelocity += lParam;
    m_nextPoint.y+=1.0f;
    return true;
}
bool Hero::onHitten(WORD wParam, DWORD lParam){
    m_bIsBusy = true;
    ControllerReceiver::getInstance()->setIsWorking(false);
    ON_HITTEN;
    m_fraction = 0.05f;
    m_hVelocity = -1*m_hVelocity;
    AnimationTool::playAnimation(m_sprite, "Hero_Hitten", CallFunc::create([this](){
        ControllerReceiver::getInstance()->setIsWorking(true);
        this->SendEvent(EVENT_RECOVER);
        m_bIsBusy = false;
    }));
    return true;
}
bool Hero::onCharge(WORD wParam, DWORD lParam)
{
    AnimationTool::playAnimation(m_sprite,"Hero_Charge");
    return true;
}

bool Hero::moveGround(WORD wParam, DWORD lParam)
{
    Point p = getCurPoint();
    p.x += m_hVelocity;
    
    m_sprite->setFlippedX(wParam ==1 ? false:true);
    setDirection(wParam ==1 ? false:true);

    setNextPoint(p);
    return true;
}

bool Hero::moveAir(WORD wParam, DWORD lParam)
{
    static float32 fGravity = 0.15f;
    
    Point p = getCurPoint();
    m_vVelocity-=fGravity;
    p.y+=m_vVelocity;
    p.x+=m_hVelocity;
    setNextPoint(p);

    if(p.y <= m_pStageBelonged->getGroundHeight()+m_boxHeight){
        this->SendEvent(EVENT_LAND, 0, 0);
    }
    return true;
}
bool Hero::die(WORD wParam, DWORD lParam)
{
    AnimationTool::playAnimation(m_sprite,"Hero_Charge");
    return true;
}

bool Hero::attack(WORD wParam, DWORD lParam)
{
    srand(time(0));
    int randX = random(0, 1000);
    char name[32];
    if(!(randX%33)){
        sprintf(name, "shout_%d.wav",randX<500?1:2);
        AudioTool::getInstance()->PlaySoundEffect(name);
    }
    m_pCombat->createAttack((SKILL_TYPE)wParam,lParam);
    return true;
}

void Hero::Update(float dt)
{
    if(ControllerReceiver::getInstance()->getIsWorking())
        updatePosition(dt);
    else{
        Player::updatePosition(dt);
    }
}

void Hero::processCollide(int collideType, Player *player){
    if(this == player || this->getType() == player->getType()){
        return;
    }
    switch(collideType){
        case 0:
        {
            const skillData * sk = player->getCompatComponent()->getCurEffect();
            if(sk){
                m_receivedEffect = sk;
            }
            setDirection(player->getDirection());
            m_sprite->setFlippedX(getDirection());
            this->SendEvent(EVENT_HITTEN);
        }
            break;
        case 1:
            break;
        case 2:
            break;
    }
}

void Hero::updatePosition(float dt){
    Point p = getNextPoint();
    if(p.y < m_pStageBelonged->getGroundHeight()+m_boxHeight){
        p.y = m_pStageBelonged->getGroundHeight()+m_boxHeight;
    }
    m_sprite->setPosition(p);
    setLastPoint(getCurPoint());
    setCurPoint(p);
    
    if(m_pCollide){
        m_pCollide->updatePosition(m_sprite->getPosition(),!m_bFaceRight);
    }
}