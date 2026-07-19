#include "LogBuffer.h"
#include "imgui.h"
#include <cstdarg>
#include <cstdio>
#include <ctime>
#include <algorithm>

namespace coco {

LogBuffer::LogBuffer(size_t maxLines) : m_maxLines(maxLines) {
    m_messages.reserve(maxLines);
}

void LogBuffer::Add(LogLevel level, const char* format, ...) {
    char buf[4096];
    va_list args;
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    time_t now = ::time(nullptr);
    struct tm local{};
    ::localtime_s(&local, &now);
    char timeStr[32];
    ::strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &local);

    std::string line = std::string("[") + timeStr + "] " + buf;
    m_messages.push_back({level, line});

    if (m_messages.size() > m_maxLines)
        m_messages.erase(m_messages.begin());
    m_scrollToBottom = true;
}

void LogBuffer::Clear() { m_messages.clear(); }

void LogBuffer::Draw(const char* title) {
    ImGui::BeginChild(title, ImVec2(0, 0), true,
                      ImGuiWindowFlags_HorizontalScrollbar);
    if (ImGui::BeginPopupContextWindow()) {
        if (ImGui::MenuItem("清空")) Clear();
        ImGui::EndPopup();
    }

    for (const auto& entry : m_messages) {
        ImVec4 color(1, 1, 1, 1);
        switch (entry.level) {
            case LogLevel::Warning: color = ImVec4(1,1,0,1); break;
            case LogLevel::Error:   color = ImVec4(1,0,0,1); break;
            case LogLevel::Debug:   color = ImVec4(0.6f,0.6f,0.6f,1); break;
            default: break;
        }
        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(entry.text.c_str());
        ImGui::PopStyleColor();
    }

    if (m_scrollToBottom && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() * 0.8f)
        ImGui::SetScrollHereY(1.0f);

    ImGui::EndChild();
}

bool LogBuffer::SaveToFile(const char* path) const {
    FILE* f = nullptr;
    if (::fopen_s(&f, path, "w") != 0 || !f) return false;
    for (const auto& e : m_messages)
        ::fprintf(f, "%s\n", e.text.c_str());
    ::fclose(f);
    return true;
}

} // namespace coco
