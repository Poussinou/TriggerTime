/*
 * Copyright (c) 2014, Stanislav Vorobiov
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "LevelCompletedComponent.h"
#include "Renderer.h"
#include "Scene.h"
#include "Settings.h"
#include "AssetManager.h"
#include "InputManager.h"
#include "Utils.h"
#include "SequentialTweening.h"
#include <boost/make_shared.hpp>

namespace af
{
    LevelCompletedComponent::LevelCompletedComponent(const std::string& thisMission,
        const std::string& nextMission,
        const std::string& scriptPath,
        const std::string& assetPath,
        int zOrder)
    : UIComponent(zOrder),
      logo_(assetManager.getImage("common1/level_completed.png")),
      itemTime_(0.0f),
      numEntries_(0),
      blackoutDone_(false),
      displayButtons_(false),
      currentItem_(-1),
      tweenTime_(0.0f),
      selectionTimeout_(0.0f),
      finger_(-1),
      fingerItem_(-1),
      scriptPath_(scriptPath),
      assetPath_(assetPath),
      strm_(audio.createStream("theme.ogg")),
      sndHit_(audio.createSound("text_hit.ogg"))
    {
        button_[0] = assetManager.getImage("common1/main_menu.png");
        button_[1] = assetManager.getImage("common1/next_level.png");

        SequentialTweeningPtr tweening = boost::make_shared<SequentialTweening>(true);

        tweening->addTweening(boost::make_shared<SingleTweening>(0.5f, EaseInOutQuad, 0.0f, 1.0f));
        tweening->addTweening(boost::make_shared<SingleTweening>(0.5f, EaseInOutQuad, 1.0f, 0.0f));

        tweening_ = tweening;

        strm_->setLoop(true);

        addEntry("Mission:", thisMission);
        addEntry("Enemies killed:", "--");
        addEntry("Accuracy:", "--%");
        addEntry("Time spent:", "--");
        addEntry("Next mission:", nextMission);
    }

    LevelCompletedComponent::~LevelCompletedComponent()
    {
    }

    void LevelCompletedComponent::accept(ComponentVisitor& visitor)
    {
        visitor.visitUIComponent(shared_from_this());
    }

    void LevelCompletedComponent::update(float dt)
    {
        if (!blackoutDone_) {
            if (stainedGlass_->color() == settings.levelCompleted.blackoutColor) {
                blackoutDone_ = true;
                strm_->play();
            }
            return;
        }

        itemTime_ += dt;

        if (numEntries_ < entries_.size()) {
            if (itemTime_ >= settings.levelCompleted.itemTimeout) {
                itemTime_ = 0.0f;
                ++numEntries_;
                sndHit_->play();
            }
        } else if (numEntries_ == entries_.size()) {
            if (itemTime_ >= settings.levelCompleted.buttonsTimeout) {
                displayButtons_ = true;
                ++numEntries_;
                sndHit_->play();
            }
        }

        if (!displayButtons_) {
            return;
        }

        tweenTime_ += dt;

        if (scene()->inputMenuUI()->havePointer()) {
            b2Vec2 point;
            bool pressed = scene()->inputMenuUI()->pressed(&point);
            bool triggered = scene()->inputMenuUI()->triggered();

            if (finger_ == 0) {
                if (pressed) {
                    if (pointInRectFromCenter(point,
                                              buttonPos_[fingerItem_],
                                              buttonWidth_[fingerItem_],
                                              buttonHeight_[fingerItem_])) {
                        if (currentItem_ < 0) {
                            tweenTime_ = 0.0f;
                        }
                        currentItem_ = fingerItem_;
                    } else {
                        currentItem_ = -1;
                    }
                } else {
                    int ci = currentItem_;
                    currentItem_ = -1;
                    finger_ = -1;
                    fingerItem_ = -1;
                    onPress(ci);
                }
            } else if (triggered) {
                if (pointInRectFromCenter(point,
                                          buttonPos_[0],
                                          buttonWidth_[0],
                                          buttonHeight_[0])) {
                    finger_ = 0;
                    fingerItem_ = currentItem_ = 0;
                    tweenTime_ = 0.0f;
                } else if (pointInRectFromCenter(point,
                                                 buttonPos_[1],
                                                 buttonWidth_[1],
                                                 buttonHeight_[1])) {
                    finger_ = 0;
                    fingerItem_ = currentItem_ = 1;
                    tweenTime_ = 0.0f;
                }
            }
        } else {
            if (currentItem_ == -1) {
                currentItem_ = 1;
            }

            selectionTimeout_ -= dt;
            if (scene()->inputMenuUI()->leftPressed()) {
                if (selectionTimeout_ <= 0.0f) {
                    if (currentItem_-- == 0) {
                        currentItem_ = 1;
                    }
                    selectionTimeout_ = 0.5f;
                    tweenTime_ = 0.0f;
                }
            } else if (scene()->inputMenuUI()->rightPressed()) {
                if (selectionTimeout_ <= 0.0f) {
                    if (++currentItem_ == 2) {
                        currentItem_ = 0;
                    }
                    selectionTimeout_ = 0.5f;
                    tweenTime_ = 0.0f;
                }
            } else if (scene()->inputMenuUI()->okPressed()) {
                onPress(currentItem_);
            } else {
                selectionTimeout_ = 0.0f;
            }
        }
    }

    void LevelCompletedComponent::render()
    {
        if (!blackoutDone_) {
            return;
        }

        renderQuad(logo_,
            b2Vec2(scene()->gameWidth() / 2.0f, scene()->gameHeight() - 2.0f),
            4.0f * logo_.aspect(), 4.0f);

        for (size_t i = 0; i < (std::min)(numEntries_, entries_.size()); ++i) {
            entries_[i].value->render();
            entries_[i].key->render();
        }

        if (displayButtons_) {
            for (int i = 0; i < 2; ++i) {
                float extra = 0.0f;

                if (i == currentItem_) {
                    extra += tweening_->getValue(tweenTime_);
                }

                renderQuad(button_[i], buttonPos_[i],
                           buttonWidth_[i] + (extra * button_[i].aspect()),
                           buttonHeight_[i] + extra);
            }
        }
    }

    void LevelCompletedComponent::onRegister()
    {
        audio.stopAll(settings.levelCompleted.blackoutTime);

        stainedGlass_ = boost::make_shared<StainedGlassComponent>(zOrder() - 1);
        stainedGlass_->setColor(settings.levelCompleted.blackoutColor,
            EaseLinear, settings.levelCompleted.blackoutTime);
        parent()->addComponent(stainedGlass_);

        b2Transform xf(b2Vec2(1.0f, scene()->gameHeight() - 4.0f - settings.levelCompleted.charSize * 1.5f),
            b2Rot(0.0f));

        for (size_t i = 0; i < entries_.size(); ++i) {
            entries_[i].key->setWidth(scene()->gameWidth());
            entries_[i].key->setTransform(xf);
            entries_[i].value->setWidth(scene()->gameWidth());
            entries_[i].value->setTransform(xf);
            xf.p.y -= settings.levelCompleted.charSize * 1.5f;
        }

        buttonHeight_[0] = 3.5f;
        buttonWidth_[0] = buttonHeight_[0] * button_[0].aspect();
        buttonPos_[0] = b2Vec2(buttonWidth_[0] / 2.0f, buttonHeight_[0] / 2.0f);
        buttonPos_[0] += b2Vec2(2.0f, 1.0f);

        buttonHeight_[1] = 3.5f;
        buttonWidth_[1] = buttonHeight_[1] * button_[1].aspect();
        buttonPos_[1] = b2Vec2(buttonWidth_[1] / 2.0f, buttonHeight_[1] / 2.0f);
        buttonPos_[1] += b2Vec2(scene()->gameWidth() - buttonWidth_[1] - 2.0f, 1.0f);
    }

    void LevelCompletedComponent::onUnregister()
    {
    }

    void LevelCompletedComponent::addEntry(const std::string& key, const std::string& value)
    {
        Entry entry;

        entry.key = boost::make_shared<TextArea>();
        entry.key->setCharSize(settings.levelCompleted.charSize);
        entry.key->setColor(settings.levelCompleted.keyColor);
        entry.key->setText(key);

        entry.value = boost::make_shared<TextArea>();
        entry.value->setCharSize(settings.levelCompleted.charSize);
        entry.value->setColor(settings.levelCompleted.valueColor);
        entry.value->setText(key + " " + value);

        entries_.push_back(entry);
    }

    void LevelCompletedComponent::renderQuad(const Image& image, const b2Vec2& pos,
        float width, float height)
    {
        renderer.setProgramDef(image.texture());

        RenderTriangleFan rop = renderer.renderTriangleFan();

        b2Vec2 tmp = pos - b2Vec2(width / 2, height / 2); rop.addVertex(tmp.x, tmp.y);
        tmp = pos + b2Vec2(width / 2, -height / 2); rop.addVertex(tmp.x, tmp.y);
        tmp = pos + b2Vec2(width / 2, height / 2); rop.addVertex(tmp.x, tmp.y);
        tmp = pos + b2Vec2(-width / 2, height / 2); rop.addVertex(tmp.x, tmp.y);

        rop.addTexCoords(image.texCoords(), 6);

        rop.addColors();
    }

    void LevelCompletedComponent::onPress(int i)
    {
        if (i == 0) {
            scene()->setNextLevel("main_menu.lua", "");
        } else if (i == 1) {
            scene()->setNextLevel(scriptPath_, assetPath_);
        }
    }
}
