#include "CommandListInserter.h"

#include "Utility/DxObjectNaming.h"

#include <cassert>

CommandListInserter::CommandListInserter(CommandQueue &commandQueue)
    : commandQueue(commandQueue) {}

CommandListInserter::~CommandListInserter() {
    assert(!hasUnsubmittedList());
}

void CommandListInserter::newList() {
    if (lists.size() > 0) {
        ensureLastListIsClosed();
    }
    lists.push_back(std::make_unique<CommandList>(commandQueue));
}

void CommandListInserter::newList(const std::wstring &name) {
    newList();
    SET_OBJECT_NAME(*lists.back(), name.c_str());
}

CommandList &CommandListInserter::currentList() {
    if (!hasUnsubmittedList()) {
        newList();
    }
    return *lists.back();
}

void CommandListInserter::submitLists() {
    std::vector<CommandList *> listsForSubmission = getListsForSubmission();
    if (listsForSubmission.size() == 0) {
        return;
    }

    ensureLastListIsClosed();
    this->firstUnsubmittedListIndex = this->lists.size();
    commandQueue.executeCommandListsAndSignal(listsForSubmission);
}

std::vector<CommandList *> CommandListInserter::getListsForSubmission() {
    const auto begin = this->lists.begin() + this->firstUnsubmittedListIndex;
    const auto end = this->lists.end();

    std::vector<CommandList *> result{};
    for (auto it = begin; it != end; it++) {
        result.push_back(it->get());
    }

    return result;
}

void CommandListInserter::ensureLastListIsClosed() {
    if (!lastListClosed) {
        lists.back()->close();
        lastListClosed = true;
    }
}

bool CommandListInserter::hasUnsubmittedList() const {
    return this->firstUnsubmittedListIndex < lists.size();
}
