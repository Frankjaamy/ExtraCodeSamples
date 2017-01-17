/*
 In the beginning, Please pardon me for my inadequate English.
 
 This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.


 This is part of the inplementation of combat component in the game Blade-Remake.  Each Player(hero and enemies) object has a combat component 
 to process the combat requests like "What action the hero or the enemy will be using? What 2D frames will be played? Between which frames will this action become effectable?" 


*/

#include "CombatSkill.h"
#include "AnimationTool.h"
#include "Player.h"
#include "ControllReceiver.h"

#define ATTACK_OVER \
    m_player->getMySprite()->stopAllActions();\
    m_player->SendEvent(EVENT_ATTACK_OVER);\
    m_player->setisPlayerBusy(false);\

/*
    This struct, attackAnimationInformation, defines all the data required - animation name, sound name, which frame is the actual beginning of 
    the attack, and which fram is the end of the attack action. (For example, in an animation of frame 0 - 10, preAttackIndex = 3, aftAttackIndex = 7 means that frame 0-3
    is the preparation of the attack, 3-7 is the effective attack action, and 7-10 is the end of this animation, the enemy will only be hitten if it has a collision with the 
    attack range during frame 3-7).
*/
struct attackAniInfo{
    char aniName[32];
    char soundName[32];
    int preAttackIndex;
    int aftAttackIndex;
    MyCollideBox attackCollide;
    skillData skillInfo;
    attackAniInfo(){;};
    attackAniInfo(skillData skill, const char * name, const char * name_sound, int preIndex, int aftIndex, MyCollideBox rect){
        skillInfo = skillData(skill);
        strcpy(aniName, name);
        strcpy(soundName,name_sound);
        preAttackIndex = preIndex;
        aftAttackIndex = aftIndex;
        attackCollide = MyCollideBox(rect);
    }
};
/*
    This was the defination of hero attack movements. Technically, I should make these data in a txt file or in an excel form and read them. 
    Here I want to make things clear and readable so I just do it this way.
*/
const attackAniInfo heroAttackInfoArray[3][6] = {
    {
        attackAniInfo(skillData(SKILL_NORMAL,5,0,0,0,0,0),"Sword_1", "blade_1.wav",3, 7,MyCollideBox(13,0,40,30)),
        attackAniInfo(skillData(SKILL_NORMAL,8,0,0,0,0.2f,0),"Sword_2", "blade_2.wav",0, 5,MyCollideBox(13,-10,50,55)),
        attackAniInfo(skillData(SKILL_NORMAL,15,10,0,0,0.3f,0),"Sword_3", "blade_3.wav",3, 7,MyCollideBox(13,-5,50,45)),
        attackAniInfo(skillData(SKILL_STUN,8,5,0,1.5f,0,3.0f),"Sword_4", "blade_2.wav",3, 7,MyCollideBox(13,-5,35,55)),
        attackAniInfo(skillData(SKILL_BLOWAWAY,20,15,0,0,2,3),"Sword_5", "blade_4.wav",3, 7,MyCollideBox(13,-20,65,64)),
        attackAniInfo(skillData(SKILL_NORMAL,5,0,0,0,0,0),"Sword_6", "blade_5.wav",1, 6,MyCollideBox(0,0,0,0)),
    }
};
const attackAniInfo enemyInfoArray[1] = {
    attackAniInfo(skillData(SKILL_NORMAL,20,0,0,0,1.0,0),"Lion_Attack","lion_roar.wav",5,9,MyCollideBox(10,0,60,35))
};

CombatSkill::CombatSkill(Player *p):
m_player(p),
m_curSkill(SKILL_SWORD_1),
m_curEffect(nullptr),
m_curSkillDamage(0),
m_curWeaponType(WEAPON_SWORD)
{
}

CombatSkill::~CombatSkill(){
    ;
}
void CombatSkill::createMonsterAttack(PLAYER_TYPE type){
    if(m_player->getisPlayerBusy()){
        return;
    }
    m_player->setisPlayerBusy(true);
    switch(type){
        case TYPE_LION:
        {
            /*
                The code here divides a whole animation into three animations.
                Animation 1 is the preparation of the attack.
                Animation 2 is the effective action of the attack.
                Animation 3 is the end of the action.
                Each animation has a callback function, processing the different game logic when one animation is played.  
            */
            int preAttackIndex = enemyInfoArray[TYPE_LION-1].preAttackIndex;
            int afterAttackIndex = enemyInfoArray[TYPE_LION-1].aftAttackIndex;
            std::string tempName(enemyInfoArray[TYPE_LION-1].aniName);
            Animation * ani1 = AnimationTool::getAnimation(tempName, 1, preAttackIndex,0.1f);
            Animation * ani2 = AnimationTool::getAnimation(tempName, preAttackIndex+1,afterAttackIndex,0.1f);
            Animation * ani3 = AnimationTool::getAnimation(tempName, afterAttackIndex+1, FIND_ANI_UNTIL_THE_END,0.1f);
            auto callAttackBegin = CallFunc::create( [&] {
                log("Actual Attack Begin");
                // MyCollideBox is a class defining an effective collision area.
                MyCollideBox *mp = new MyCollideBox(enemyInfoArray[TYPE_LION-1].attackCollide);
                mp->boxOwner = m_player;
                // In function createBoxes, the memory that mp points was managed by the collision manager. 
                m_player->getCollideIndicator()->createBoxes(false,mp);
                // During effective action, the effect of this action (power, buff, debuff,etc) will be saved.
                m_curEffect = &(enemyInfoArray[TYPE_LION-1].skillInfo);
                
                AudioTool::getInstance()->PlaySoundEffect(enemyInfoArray[TYPE_LION-1].soundName);
            });
            auto callAttackFinish = CallFunc::create( [&] {
                // the attack action is over, so we so cleanning work.
                log("Actual Attack Finish");
                m_player->setisPlayerBusy(false);
                m_player->getCollideIndicator()->removeAllActionBoxes();
                m_curEffect = nullptr;
            });
            auto callAttackEnd = CallFunc::create( [&] {
                log("phase3");
                ATTACK_OVER
            });

            FiniteTimeAction * action = Sequence::create
            (
             Animate::create(ani1),
             callAttackBegin,
             Animate::create(ani2),
             callAttackFinish,
             Animate::create(ani3),
             callAttackEnd,
             NULL
             );
            m_player->getMySprite()->stopAllActions();
            m_player->getMySprite()->runAction(action);
        }
            break;
        default:
            break;
    }
}
/*
    This function is for player attack movements, it a little more complicated but the idea does change.
*/

void CombatSkill::createAttack(SKILL_TYPE whichSKill,int combatMode){
    if(m_player->getisPlayerBusy()){
        return;
    }
    
    switch (combatMode) {
        case 0:
            break;
        default:
                m_curSkill = whichSKill;
                m_curEffect = &heroAttackInfoArray[m_curWeaponType][m_curSkill].skillInfo;
                if(m_player->getMP()<m_curEffect->manaCost){
                    log("Not enough mana!");
                    return;
                }
            break;
    }

    m_player->setisPlayerBusy(true);
    if(m_player->getDirection()){
        m_player->getMySprite()->setAnchorPoint(Point(0.75,0));
    }
    else{
        m_player->getMySprite()->setAnchorPoint(Point(0.25,0));
    }
    int preAttackIndex = heroAttackInfoArray[m_curWeaponType][m_curSkill].preAttackIndex;
    int afterAttackIndex = heroAttackInfoArray[m_curWeaponType][m_curSkill].aftAttackIndex;
    std::string tempName(heroAttackInfoArray[m_curWeaponType][m_curSkill].aniName);
    
    Animation * ani1 = AnimationTool::getAnimation(tempName, 1, preAttackIndex,0.1f);
    Animation * ani2 = AnimationTool::getAnimation(tempName, preAttackIndex+1,afterAttackIndex,0.1f);
    Animation * ani3 = AnimationTool::getAnimation(tempName, afterAttackIndex+1, FIND_ANI_UNTIL_THE_END,0.1f);
    auto callAttackBegin = CallFunc::create( [&] {
        log("Actual Attack Begin");
        
        MyCollideBox *mp = new MyCollideBox(heroAttackInfoArray[m_curWeaponType][m_curSkill].attackCollide);
        mp->boxOwner = m_player;
        m_player->getCollideIndicator()->createBoxes(false,mp);
        
        m_curEffect = &heroAttackInfoArray[m_curWeaponType][m_curSkill].skillInfo;
        int mana = m_player->getMP();
        m_player->setMP(mana-m_curEffect->manaCost);
        
        AudioTool::getInstance()->PlaySoundEffect(heroAttackInfoArray[m_curWeaponType][m_curSkill].soundName);
    });
    auto callAttackFinish = CallFunc::create( [&] {
        log("Acutal Attack Finish");
        short sk = (short)m_curSkill;
        sk = (++sk)%3;
        m_curSkill = (SKILL_TYPE)sk;
        m_player->setisPlayerBusy(false);
        
        m_player->getCollideIndicator()->removeAllActionBoxes();
        m_curEffect = nullptr;
    });
    auto callAttackEnd = CallFunc::create( [&] {
        log("phase3");
        ATTACK_OVER
        m_curSkill = (SKILL_TYPE)0;
    });
    FiniteTimeAction * action = Sequence::create
    (
        Animate::create(ani1),
        callAttackBegin,
        Animate::create(ani2),
        callAttackFinish,
        Animate::create(ani3),
        callAttackEnd,
        NULL
    );
    m_player->getMySprite()->stopAllActions();
    m_player->getMySprite()->runAction(action);
}