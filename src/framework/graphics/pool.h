/*
 * Copyright (c) 2010-2020 OTClient <https://github.com/edubart/otclient>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef POOL_H
#define POOL_H

#include "declarations.h"
#include "framebuffer.h"
#include "texture.h"
#include "framebuffermanager.h"

class Pool
{
public:
    void setEnable(const bool v) { m_enabled = v; }
    bool isEnabled() const { return m_enabled; }

protected:
    enum class DrawMethodType {
        DRAW_FILL_COORDS,
        DRAW_TEXTURE_COORDS,
        DRAW_TEXTURED_RECT,
        DRAW_UPSIDEDOWN_TEXTURED_RECT,
        DRAW_REPEATED_TEXTURED_RECT,
        DRAW_REPEATED_FILLED_RECT,
        DRAW_FILLED_RECT,
        DRAW_FILLED_TRIANGLE,
        DRAW_BOUNDING_RECT
    };

    struct DrawMethod {
        DrawMethodType type;
        std::pair<Rect, Rect> rects{};
        std::tuple<Point, Point, Point> points{};
        Point dest{};
        uint64 intValue{ 0 };
        float floatValue{ .0f };
    };

    struct DrawObject {
        ~DrawObject() { drawMethods.clear(); coordsBuffer = nullptr; state.texture = nullptr; action = nullptr; }

        Painter::PainterState state;
        std::shared_ptr<CoordsBuffer> coordsBuffer;
        Painter::DrawMode drawMode{ Painter::DrawMode::Triangles };
        std::vector<DrawMethod> drawMethods;

        std::function<void()> action{ nullptr };
    };

private:
    struct State {
        Painter::CompositionMode compositionMode;
        Rect clipRect;
        float opacity;
        bool alphaWriting{ true };
    };

    void setCompositionMode(const Painter::CompositionMode mode, const int pos = -1);
    void setClipRect(const Rect& clipRect, const int pos = -1);
    void setOpacity(const float opacity, const int pos = -1);

    void resetClipRect() { m_state.clipRect = Rect(); }
    void resetCompositionMode() { m_state.compositionMode = Painter::CompositionMode_Normal; }
    void resetOpacity() { m_state.opacity = 1.f; }
    void resetState() { resetClipRect(); resetCompositionMode(); resetOpacity(); }
    void startPosition() { m_indexToStartSearching = m_objects.size(); }

    virtual bool hasFrameBuffer() const { return false; };
    virtual FramedPool* toFramedPool() { return nullptr; }

    std::vector<DrawObject> m_objects;

    bool m_enabled{ true };
    State m_state;

    uint16_t m_indexToStartSearching{ 0 };

    friend class DrawPool;
};

class FramedPool : public Pool {
public:
    void onBeforeDraw(std::function<void()> f) { m_beforeDraw = f; }
    void onAfterDraw(std::function<void()> f) { m_afterDraw = f; }
    void resize(const Size& size) { m_framebuffer->resize(size); }
    void setSmooth(bool enabled) { m_framebuffer->setSmooth(enabled); }

protected:
    friend class DrawPool;

private:
    void updateStatus() { m_status.first = m_status.second; }
    void resetCurrentStatus() { m_status.second = 0; }
    bool hasModification() const { return m_status.first != m_status.second; }
    bool hasFrameBuffer() const override { return m_framebuffer != nullptr; }

    FramedPool* toFramedPool() override { return static_cast<FramedPool*>(this); }

    FrameBufferPtr m_framebuffer;
    Rect m_dest, m_src;

    std::function<void()> m_beforeDraw, m_afterDraw;
    std::pair<size_t, size_t> m_status{ 0,0 };
};

extern DrawPool g_drawPool;

#endif
