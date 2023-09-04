#include "SanityBeforeStagePlugin.h"

#include <regex>

#include "Config/TaskData.h"
#include "Controller/Controller.h"
#include "Utils/ImageIo.hpp"
#include "Utils/Logger.hpp"
#include "Vision/RegionOCRer.h"

bool asst::SanityBeforeStagePlugin::verify(AsstMsg msg, const json::value& details) const
{
    // SubTaskStart会在主任务的操作部分之前触发，想不通，但是如果哪天改了记得来这里修
    if (msg != AsstMsg::SubTaskStart || details.get("subtask", std::string()) != "ProcessTask") {
        return false;
    }

    const std::string task = details.at("details").at("task").as_string();
    if (task == "StartButton1" || task == "Fight@StartButton1") {
        return true;
    }
    else {
        return false;
    }
}

bool asst::SanityBeforeStagePlugin::_run()
{
    LogTraceFunction;

    get_sanity();

    return true;
}

/// <summary>
/// 获取 当前理智/最大理智
/// </summary>
void asst::SanityBeforeStagePlugin::get_sanity()
{
    LogTraceFunction;

    RegionOCRer analyzer(ctrler()->get_image());
    analyzer.set_task_info("SanityMatch");

    if (!analyzer.analyze()) {
        Log.info(__FUNCTION__, "Current Sanity analyze failed");
        return;
    }
    std::string text = analyzer.get_result().text;

    if (!text.find('/') && text.length() > 2) {
        if (text[text.length() - 3] == '1' && text[text.length() - 2] >= '0' && text[text.length() - 2] <= '3') {
            text = text.substr(0, text.length() - 3) + '/' + text.substr(text.length() - 3);
        }
        else {
            text = text.substr(0, text.length() - 2) + '/' + text.substr(text.length() - 2);
        }
    }
    Log.info(__FUNCTION__, "Current Sanity:" + text);

    json::value sanity_info = basic_info_with_what("SanityBeforeStage");
    sanity_info["details"]["sanity"] = text;
    callback(AsstMsg::SubTaskExtraInfo, sanity_info);
}
