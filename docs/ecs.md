# Entity-Component-System (ECS)

Entity-component-system (ECS) design philosophies make up the backbone of the
core Mondradiko engine.

ECS is defined by three major terms:

- entity: an opaque identifier that contains no state.
- component: a piece of functionless state that may be attached to an entity.
- system: a stateless function that operates on sets of components.

If you're unfamiliar with ECS as a concept, or its consequences in broader game
engine design, here are some helpful learning resources:

- [Robert Nystrom on the "Component" pattern](https://gameprogrammingpatterns.com/component.html)
- [Mick West - Evolve Your Hierarchy](http://cowboyprogramming.com/2007/01/05/evolve-your-heirachy/)
- [A-Frame on its entity-component-system](https://github.com/aframevr/aframe/blob/master/docs/introduction/entity-component-system.md)
- [Bob Nystrom - Is There More to Game Architecture than ECS?](https://www.youtube.com/watch?v=JxI3Eu5DPwE)
- [Overwatch Gameplay Architecture and Netcode](https://www.youtube.com/watch?v=W3aieHjyNvw)

# Implementation

## Library

## The ECS World

## Component Types

## Singleton Components

## The Pipeline

## Modules

## Network Synchronization

## Scripting Interface

# Why ECS?

## Why not OOP?

Object-oriented programming (OOP) may be more familiar to more application
developers, front-end designers, and game programmers (but not game engine
programmers), but OOP has many, many faults that can make engineering game
engines harder than is practical. The next sections about the benefits of ECS
will go into more detail on why, when, and where certain OOP philosophies fail
in the context of game engine architecture.

# Design Goals

ECS brings a number of social and philosophical benefits to Mondradiko.

## Simplicity

## Decoupling

## Performance

## Familiarity

Anyone who has worked with the Unity engine or Unreal Engine will have at the
very least a basic understanding of core ECS concepts. ECS is also a very
well-established design philosophy that's been proven to be effective time and
time again in many different game design contexts. Entity-component-system
*works* in game development, from AAA production to indie, and tens of thousands
of skilled game developers can already express themselves effectively through
it.

# Problems Solved

There are a lot of engineering challenges facing Mondradiko, that have either
already been confronted by the prototype engine, or will have to be newly faced
in this iteration. Entity-component-system design approaches can theoretically
solve a lot of these challenges.

## Scripting API Design

## Network Synchronization

## Threading

## Resource Lifetime

## Deep Inheritance
