#pragma once

#include "CommandList/CommandList.h"

#include <DXD/Utility/NonCopyableAndMovable.h>
#include <string>
#include <vector>
#include <memory>

class CommandList;
class CommandQueue;

class CommandListInserter : DXD::NonCopyableAndMovable {
public:
    CommandListInserter(CommandQueue &commandQueue);
    ~CommandListInserter();

    void newList();
    void newList(const std::wstring &name);
    CommandList &currentList();
    void submitLists();

private:
    std::vector<CommandList *> getListsForSubmission();
    void ensureLastListIsClosed();
    bool hasUnsubmittedList() const;

    CommandQueue &commandQueue;
    std::vector<std::unique_ptr<CommandList>> lists{};
    size_t firstUnsubmittedListIndex = 0u;
    bool lastListClosed = false;
};
