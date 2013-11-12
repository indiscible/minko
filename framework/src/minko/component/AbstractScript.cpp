/*
Copyright (c) 2013 Aerys

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "AbstractScript.hpp"

#include "minko/scene/Node.hpp"
#include "minko/component/SceneManager.hpp"

using namespace minko;
using namespace minko::component;

void
AbstractScript::initialize()
{
	_targetAddedSlot = targetAdded()->connect(std::bind(
		&AbstractScript::targetAddedHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2
	));

	_targetRemovedSlot = targetRemoved()->connect(std::bind(
		&AbstractScript::targetRemovedHandler, shared_from_this(), std::placeholders::_1, std::placeholders::_2
	));
}

void
AbstractScript::targetAddedHandler(AbstractComponent::Ptr cmp, scene::Node::Ptr target)
{
	_componentAddedSlot = target->componentAdded()->connect(std::bind(
		&AbstractScript::componentAddedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	_componentRemovedSlot = target->componentRemoved()->connect(std::bind(
		&AbstractScript::componentRemovedHandler,
		shared_from_this(),
		std::placeholders::_1,
		std::placeholders::_2,
		std::placeholders::_3
	));

	_started[target] = false;
}

void
AbstractScript::targetRemovedHandler(AbstractComponent::Ptr cmp, scene::Node::Ptr target)
{
	_componentAddedSlot = nullptr;
	_componentAddedSlot = nullptr;
}

void
AbstractScript::componentAddedHandler(scene::Node::Ptr			node,
									  scene::Node::Ptr			target,
									  AbstractComponent::Ptr	component)
{
	auto sceneManager = std::dynamic_pointer_cast<SceneManager>(component);

	if (sceneManager)
		setSceneManager(sceneManager);
}

void
AbstractScript::componentRemovedHandler(scene::Node::Ptr		node,
										scene::Node::Ptr		target,
										AbstractComponent::Ptr	component)
{
	auto sceneManager = std::dynamic_pointer_cast<SceneManager>(component);

	if (sceneManager)
		setSceneManager(nullptr);
}

void
AbstractScript::frameBeginHandler(SceneManager::Ptr sceneManager)
{
	for (auto& target : targets())
	{
		if (!_started[target])
		{
			_started[target] = true;

			start(target);
		}

		update(target);
	}
}

void
AbstractScript::setSceneManager(SceneManager::Ptr sceneManager)
{
	if (sceneManager)
	{
		_frameBeginSlot = sceneManager->frameBegin()->connect(std::bind(
			&AbstractScript::frameBeginHandler, shared_from_this(), std::placeholders::_1
		));
	}
	else if (_frameBeginSlot)
	{
		for (auto& target : targets())
		{
			_started[target] = false;
			stop(target);
		}

		_frameBeginSlot = nullptr;
	}
}