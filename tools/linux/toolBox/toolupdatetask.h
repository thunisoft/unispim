#ifndef TOOLAUTOUPDATETASK_H
#define TOOLAUTOUPDATETASK_H
#include <QThread>
#include "commondef.h"


class ToolUpdateTask : public QThread
{
    Q_OBJECT

protected:
    virtual void run() override;

public:
    void set_update_type(UPDATE_TYPE input_update_type);
    void set_tool_addon_info(TOOL_ADDON_INFO input_addon_info);

private:
    void handle_inform_update_tool(const SEVER_ADDON_INFO& addon_info);
    void handle_force_update_tool(const SEVER_ADDON_INFO& addon_info);
    void handle_deprecated_tool(const SEVER_ADDON_INFO& addon_info);
    void handle_publish_tool(const SEVER_ADDON_INFO& addon_info);


private:
    void HandleManuTask(SEVER_ADDON_INFO server_addon_info);
    void HandleAutoTask(SEVER_ADDON_INFO server_addon_info);
    void FilterCachedOldVersionAddon(TOOL_ADDON_INFO addon_index);
    void RemoveAddonFile(SEVER_ADDON_INFO addon_index);

signals:
    void update_tool_info();
    void download_progress(qint64 bytesReceived,qint64 bytesTotal);
    void has_no_update();
    void check_failed();

public slots:
    void slot_handle_download_progress(qint64 receiveBytes, qint64 totalBytes);
private:
    UPDATE_TYPE m_task_type = UPDATE_TYPE::AutoUpdate;
    TOOL_ADDON_INFO m_single_addon_info;
};

#endif // TOOLAUTOUPDATETASK_H
