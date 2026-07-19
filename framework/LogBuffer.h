#pragma once

#include "../core/Core.h"
#include <string>
#include <vector>

namespace coco {

// ==============================
// LogBuffer — 环形缓冲区日志
// ==============================
class LogBuffer {
public:
    explicit LogBuffer(size_t maxLines = 200);

    void Add(LogLevel level, const char* format, ...);

    void Draw(const char* title = "日志");
    void Clear();
    bool SaveToFile(const char* path) const;

    const auto& Messages() const { return m_messages; }

private:
    struct Entry {
        LogLevel level;
        std::string text;
    };

    std::vector<Entry> m_messages;
    size_t m_maxLines;
    bool   m_scrollToBottom = true;
};

} // namespace coco
