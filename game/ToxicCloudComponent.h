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

#ifndef _TOXICCLOUDCOMPONENT_H_
#define _TOXICCLOUDCOMPONENT_H_

#include "PhasedComponent.h"
#include "ParticleEffectComponent.h"
#include "PointLight.h"
#include "Tweening.h"
#include "AudioManager.h"
#include <boost/enable_shared_from_this.hpp>

namespace af
{
    class ToxicCloudComponent : public boost::enable_shared_from_this<ToxicCloudComponent>,
                                public PhasedComponent
    {
    public:
        ToxicCloudComponent(const ParticleEffectComponentPtr& pec, float distance, float duration1, float duration2,
            float damage, float damageTimeout, float effectDuration);
        ~ToxicCloudComponent();

        virtual ComponentPtr sharedThis() { return shared_from_this(); }

        virtual void accept(ComponentVisitor& visitor);

        virtual void update(float dt);

        virtual void debugDraw();

    private:
        virtual void onRegister();

        virtual void onUnregister();

        b2Vec2 damageCenter() const;

        float damageRadius() const;

        ParticleEffectComponentPtr pec_;
        PointLightPtr light_;
        TweeningPtr tweening_;
        float tweenTime_;
        float damageT_;
        float effectT_;
        float effectDuration_;

        float damage_;
        float damageTimeout_;
        AudioSourcePtr snd_;
    };

    typedef boost::shared_ptr<ToxicCloudComponent> ToxicCloudComponentPtr;
}

#endif
