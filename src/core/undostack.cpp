#include "undostack.h"
#include "operation.h"

UndoStack::UndoStack(QObject *parent) : QObject(parent) {}

void UndoStack::push(Operation *operation)
{
    m_undoStack.append(operation);
    m_redoStack.clear();
    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit stateChanged();
}

bool UndoStack::canUndo() const
{
    return !m_undoStack.isEmpty();
}

bool UndoStack::canRedo() const
{
    return !m_redoStack.isEmpty();
}

void UndoStack::undo()
{
    if (!canUndo()) return;

    Operation *op = m_undoStack.takeLast();
    m_redoStack.append(op);
    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit stateChanged();
}

void UndoStack::redo()
{
    if (!canRedo()) return;

    Operation *op = m_redoStack.takeLast();
    m_undoStack.append(op);
    emit canUndoChanged(canUndo());
    emit canRedoChanged(canRedo());
    emit stateChanged();
}

void UndoStack::clear()
{
    qDeleteAll(m_undoStack);
    qDeleteAll(m_redoStack);
    m_undoStack.clear();
    m_redoStack.clear();
    emit canUndoChanged(false);
    emit canRedoChanged(false);
    emit stateChanged();
}

const Operation *UndoStack::undoOperation() const
{
    return m_undoStack.isEmpty() ? nullptr : m_undoStack.last();
}

const Operation *UndoStack::redoOperation() const
{
    return m_redoStack.isEmpty() ? nullptr : m_redoStack.last();
}