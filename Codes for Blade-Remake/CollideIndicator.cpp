//
//  CollideIndicator.cpp
//  Blade
//
//  Created by ChenYanNan on 15/12/18.
//
//

#include "CollideIndicator.h"
#include "Player.h"

/*

 In the beginning, Please pardon me for my inadequate English.
 
 This comment was added to provide more information to professors or other personnel
 who might receive this sample of code to evaluate my ability.


 This is a code sample containing three classes.

 1 CollideIndicator: it is a component class of which each game actor has an object. It manages and operates 
 all the active collision 2D boxes 

 2 CollideManager : a Singleton that manages all CollideIndicator objects that are being used

 3 SimpleQuadraCollideTree : a quad tree to manage and optimize game collision events. It divides the game space into different areas. When updating the game, 
   if two collision boxes are in different children, then the posible collision between these two will not be examined, saving lots of calculations and improving
   game performance especially when there are hundreds of or even thousands of collision boxes in the game scene.

   However there is a potential decrease in efficiency in my Quad Tree class : if all the game objects are not averagelly scattered -- if most of them are in one Quadrant 
   of the game scene, it will cause the Quad Tree to have a branch who has a deep level, which significantly reduces the searching efficiency. I will try to improve this later.

 From this piece of code, I hope to show my understanding of data structure, basic game algorithm, as well as C++ skill.
*/

CollideIndicator::CollideIndicator(){
    m_index = (int)CollideManager::getInstance()->addIndicator(this);
}

CollideIndicator::~CollideIndicator(){
     CollideManager::getInstance()->deleteIndicator(this);
    for(auto iter : m_bodyBoxes){
        MyCollideBox * mcb = (MyCollideBox *)iter;
        CC_SAFE_DELETE(mcb);
    }
    m_bodyBoxes.clear();
    
    for(auto iter : m_actionBoxes){
        MyCollideBox * mcb = (MyCollideBox *)iter;
        CC_SAFE_DELETE(mcb);
    }
    m_actionBoxes.clear();
    
}
void CollideIndicator::updatePosition(Point curPosition,bool faceRight)
{
    for(auto iter : m_bodyBoxes){
        MyCollideBox * rect = (MyCollideBox *)iter;
        rect->box.origin.x = curPosition.x-rect->box.size.width/2;
        rect->box.origin.y = curPosition.y+rect->box.size.height/2;
    }
    for(auto iter : m_actionBoxes){
        MyCollideBox * rect = (MyCollideBox *)iter;
        if(faceRight){
            rect->box.origin.x = curPosition.x+rect->offSetX;
            rect->box.origin.y = curPosition.y+rect->box.size.height/2+rect->offSetY;
        }else{
            rect->box.origin.x = curPosition.x-rect->offSetX-rect->box.size.width;
            rect->box.origin.y = curPosition.y+rect->box.size.height/2+rect->offSetY;
        }
    }
}
void CollideIndicator::createBoxes(bool isBodyBoxes, MyCollideBox * rect1)
{
    MyCollideBox * mp = rect1;
    do{
        if (isBodyBoxes) {
            m_bodyBoxes.push_back(mp);
        }else{
            m_actionBoxes.push_back(mp);
        }
    }while(0);
}
void CollideIndicator::removeAllActionBoxes(){
    for(auto iter : m_actionBoxes){
        MyCollideBox * r = iter;
        CC_SAFE_DELETE(r);
    }
    m_actionBoxes.clear();
}

CollideManager * CollideManager::getInstance(){
    static CollideManager cm;
    return &cm;
}

void CollideManager::init(cocos2d::Rect initBounds){
    m_pQuadTree = new SimpleQuadraCollideTree(0,initBounds);
}

 unsigned long CollideManager::addIndicator(CollideIndicator * indicator){
    m_allIndicators.push_back(indicator);
    return m_allIndicators.size()-1;
}
void CollideManager::deleteIndicator(CollideIndicator * c){
    auto iter = m_allIndicators.begin();
    for(;iter!=m_allIndicators.end();)
    {
        CollideIndicator * ci = (CollideIndicator *)(*iter);
        if (ci == c) {
            iter = m_allIndicators.erase(iter);
        }else{
            iter++;
        }
    }
}
void CollideManager::updateCollideTree(){
    m_pQuadTree->clearTree();
    std::vector<MyCollideBox *> m_allActiveBoxes;
    std::vector<MyCollideBox *> m_returnBoxes;
    for(auto iter : m_allIndicators){
        CollideIndicator * ci= (CollideIndicator *)iter;
        for(auto iter1 : ci->getBodyBoxes()){
            MyCollideBox * mc = (MyCollideBox *)iter1;
            m_pQuadTree->insert(mc);
            m_allActiveBoxes.push_back(mc);
        }
        for(auto iter2 : ci->getActionBoxes()){
            MyCollideBox * mc = (MyCollideBox *)iter2;
            m_pQuadTree->insert(mc);
            m_allActiveBoxes.push_back(mc);
        }
    }
    for(int i=0;i<m_allActiveBoxes.size();i++){
        m_returnBoxes.clear();
        MyCollideBox * mcb = m_allActiveBoxes.at(i);
        m_pQuadTree->getPossibleCollides(m_returnBoxes, mcb);
        
        //hit
        if(mcb->boxType == 1 && mcb->active){
            for(auto iter : m_returnBoxes){
                MyCollideBox * hittenBox = (MyCollideBox *)iter;
                if(hittenBox->boxType == 0){
                    if(hittenBox->box.intersectsRect(mcb->box)){
                        hittenBox->boxOwner->processCollide(0,mcb->boxOwner);
                    }
                }
            }
        }
    }
}
SimpleQuadraCollideTree::SimpleQuadraCollideTree(int level, Rect bounds){
    m_curLevel = level;
    m_curBounds = bounds;
    for (int i=0; i<4; i++) {
        nodes[i] = nullptr;
    }
}
SimpleQuadraCollideTree::~SimpleQuadraCollideTree(){
    
}
void SimpleQuadraCollideTree::clearTree()
{
    m_objects.clear();
    for(int i=0;i<4;i++){
        if(nodes[i]!=nullptr){
            nodes[i]->clearTree();
        }
    }
}

void SimpleQuadraCollideTree::split()
{
    float halfWidth = m_curBounds.size.width/2;
    float halfHeight = m_curBounds.size.height/2;
    float x = m_curBounds.origin.x;
    float y = m_curBounds.origin.y;
    
    nodes[0] = new SimpleQuadraCollideTree(m_curLevel+1,Rect(x+halfWidth,y,halfWidth,halfHeight));
    nodes[1] = new SimpleQuadraCollideTree(m_curLevel+1,Rect(x,y,halfWidth,halfHeight));
    nodes[2] = new SimpleQuadraCollideTree(m_curLevel+1,Rect(x,y+halfHeight,halfWidth,halfHeight));
    nodes[3] = new SimpleQuadraCollideTree(m_curLevel+1,Rect(x+halfWidth,y+halfHeight,halfWidth,halfHeight));
}

int SimpleQuadraCollideTree::getIndex(MyCollideBox *rect)
{
    int index = -1;
    double midpointX = m_curBounds.origin.x + (m_curBounds.size.width/ 2);
    double midpointY = m_curBounds.origin.y + (m_curBounds.size.height/ 2);
    
    Rect curRect = rect->box;
    bool firstTwoQuadrant = (curRect.origin.y< midpointY && curRect.origin.y + curRect.size.height < midpointY);
    bool lastTwoQuadrant = (curRect.origin.y< midpointY );
    if (curRect.origin.x < midpointX && curRect.origin.x+curRect.size.width < midpointX) {
        if (firstTwoQuadrant) {
            index = 1;
        }
        else if (lastTwoQuadrant) {
            index = 2;
        }
    }
    else if (curRect.origin.x > midpointX) {
        if (firstTwoQuadrant) {
            index = 0;
        }
        else if (lastTwoQuadrant) {
            index = 3;
        }
    }
    return index;
}
void SimpleQuadraCollideTree::insert(MyCollideBox *box)
{
    if (nodes[0]!=nullptr) {
        int index = getIndex(box);
        if(index != -1){
            nodes[index]->insert(box);
            return;
        }
    }
    m_objects.push_back(box);
    if(m_objects.size() >  MAX_OBJECTS && m_curLevel < MAX_LEVELS){
        split();
    }
    
    int i=0;
    while (i<m_objects.size()) {
        int index = getIndex(m_objects.at(i));
        if(index != -1 && nodes[index]){
            nodes[index]->insert(m_objects.at(i));
            auto iter = m_objects.begin() + i;
            m_objects.erase(iter);
        }
        else{
            i++;
        }
    }
}

void SimpleQuadraCollideTree::getPossibleCollides(std::vector<MyCollideBox *> & objects, MyCollideBox *box)
{
    int index = getIndex(box);
    if(index != -1 && nodes[0]!=nullptr){
        nodes[index]->getPossibleCollides(objects, box);
    }
    
    for (auto iter : m_objects) {
        MyCollideBox * rect = (MyCollideBox *)iter;
        objects.push_back(rect);
    }
    return;
}

