#ifndef _EDITOR_ACTION_H_
#define  _EDITOR_ACTION_H_

class LevelEditor;

class EditorAction {
public:
	EditorAction() : done(true) {}

	virtual ~EditorAction() = default;

	EditorAction(EditorAction const&) = default;
	
	EditorAction& operator=(EditorAction const&) = default;

	void redo(LevelEditor &editor) {
		redoImpl(editor);
		done = true;
	}

	void undo(LevelEditor &editor) {
		undoImpl(editor);
		done = false;
	}

	bool isDone() const { return done; }

private:
	bool done; // We cannot undo if already done...

	virtual void redoImpl(LevelEditor &editor) const;
	virtual void undoImpl(LevelEditor &editor) const;
};

#endif
