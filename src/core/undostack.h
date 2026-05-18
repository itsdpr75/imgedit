#ifndef UNDOSTACK_H
#define UNDOSTACK_H

#include <QObject>
#include <QVector>
#include <QUndoCommand>
#include <QUndoStack>

class Operation;

class UndoStack : public QObject
{
    Q_OBJECT

public:
    explicit UndoStack(QObject *parent = nullptr);

    void push(Operation *operation);
    bool canUndo() const;
    bool canRedo() const;
    void undo();
    void redo();
    void clear();

    const Operation *undoOperation() const;
    const Operation *redoOperation() const;

signals:
    void canUndoChanged(bool canUndo);
    void canRedoChanged(bool canRedo);
    void stateChanged();

private:
    QVector<Operation*> m_undoStack;
    QVector<Operation*> m_redoStack;
};

#endif