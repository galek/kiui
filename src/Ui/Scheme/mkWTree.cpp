//  Copyright (c) 2015 Hugo Amiard hugo.amiard@laposte.net
//  This software is provided 'as-is' under the zlib License, see the LICENSE.txt file.
//  This notice and the license may not be removed or altered from any source distribution.

#include <Ui/mkUiConfig.h>
#include <Ui/Scheme/mkWTree.h>

#include <Ui/Form/mkForm.h>
#include <Ui/Form/mkWidgets.h>

#include <Ui/Scheme/mkWExpandbox.h>

#include <Ui/Frame/mkFrame.h>

using namespace std::placeholders;

namespace mk
{
	WTreeNodeHeader::WTreeNodeHeader(const Trigger& trigger)
		: WWrapButton(nullptr, styleCls(), trigger)
	{}

	WTreeNodeBody::WTreeNodeBody()
		: Sheet(styleCls())
	{}

	WTreeNodeToggle::WTreeNodeToggle(const Trigger& triggerOn, const Trigger& triggerOff, bool on)
		: WToggle(styleCls(), triggerOn, triggerOff, on)
	{}

	WTreeNode::WTreeNode(Form* form, const string& image, const string& title, bool collapsed)
		: WExpandbox(form, title, collapsed)
		, mImage(image)
	{
		mType = cls();
		mStyle = styleCls();
	}

	void WTreeNode::build()
	{
		Sheet::build();
		mHeader = this->makeappend<WTreeNodeHeader>(std::bind(&WTreeNode::selected, this));
		mContainer = this->makeappend<WTreeNodeBody>();

		mExpandButton = mHeader->makeappend<WTreeNodeToggle>(std::bind(&WExpandbox::expand, this), std::bind(&WExpandbox::collapse, this), !mCollapsed);
		if(!mImage.empty())
			mIcon = mHeader->makeappend<WIcon>(mImage);
		mTitleLabel = mHeader->makeappend<WTitle>(mTitle);

		mExpandButton->toggleState(DISABLED);
		mContainer->hide();
	}

	Widget* WTreeNode::vappend(unique_ptr<Widget> widget)
	{
		if(mContainer->count() == 0)
			mExpandButton->toggleState(DISABLED);

		return WExpandbox::vappend(std::move(widget));
	}

	unique_ptr<Widget> WTreeNode::vrelease(Widget* widget)
	{
		if(mContainer->count() == 1)
			mExpandButton->toggleState(DISABLED);

		return WExpandbox::vrelease(widget);
	}

	void WTreeNode::selected()
	{
		Sheet* parent = mParent;
		while(parent->type() != WTree::cls())
			parent = parent->parent();

		parent->as<WTree>()->select(this);
	}

	WTree::WTree(Form* form, const Trigger& trigger)
		: ScrollSheet(styleCls(), form)
		, mRootNode(nullptr)
		, mSelected(nullptr)
		, mOnSelected(trigger)
	{
		mType = cls();
	}

	WTree::~WTree()
	{}

	void WTree::select(WTreeNode* selected)
	{
		if(mSelected)
			mSelected->header()->toggleState(ACTIVATED);

		selected->header()->toggleState(ACTIVATED);
		mSelected = selected;

		/*Widget* node = selected;
		while(node != this)
		{
			if(node->type() == WTreeNode::cls())
			{
				node->as<WTreeNode>()->expand();
				for(auto& widget : node->parent()->contents())
					if(widget.get() != node)
						widget->as<WTreeNode>()->collapse();
			}
			node = node->parent();
		}*/

		mOnSelected(selected);
	}

	/*TreeNode::TreeNode(const string& image, const string& title, bool collapsed)
		: Expandbox(title, collapsed)
	{}*/

	TreeNode::TreeNode(Object* object, Tree* tree, const string& name, bool collapsed)
		: Form(nullptr, "", [this, collapsed]() { return make_unique<WTreeNode>(this, "", this->name(), collapsed); })
		//: Form("formnode", "Form " + form->style(), [this]() { return make_unique<WTreeNode>(this, "", this->label()); })
		, mObject(object)
		, mTree(tree)
	{
		this->setName(name);
		mTree->addNode(mObject, this);
	}

	TreeNode::~TreeNode()
	{
		mTree->removeNode(mObject, this);
	}

	TableNode::TableNode(Object* object, Tree* tree, bool collapsed)
		: Form(nullptr, "", [this]() { return make_unique<WTreeNode>(this, "", this->name()); })
		, mObject(object)
		, mTree(tree)
	{}

	TableNode::~TableNode()
	{}

	Tree::Tree(std::function<void(Object*)> onSelected, Style* style)
		: Form(style ? style : styleCls(), "", [this]() { return make_unique<WTree>(this, std::bind(&Tree::selected, this, _1)); })
		, mOnSelected(onSelected)
	{}

	Tree::~Tree()
	{
		mContents.clear();
	}

	void Tree::selected(Widget* widget)
	{
		if(mOnSelected)
			mOnSelected(widget->as<WTreeNode>()->form()->as<TreeNode>()->object());
	}

	void Tree::select(Object* object)
	{
		mWidget->as<WTree>()->select(mNodes[object]->widget()->as<WTreeNode>());
	}

	void Tree::addNode(Object* object, TreeNode* node)
	{
		mNodes[object] = node;
	}

	void Tree::removeNode(Object* object, TreeNode* node)
	{
		mNodes.erase(object);
	}
}
