#pragma once

// GOAL:
//   Create a class that implements the following interface. Use your best
//     judgement when it comes to design tradeoffs and implementation decisions.
//   You may create any number of other classes to support your implementation.
//   If you alter the given code, please maintain the intent of the original code
//     and document why your alterations were necessary.

enum class AttributeKey
{
    Invalid = 0,
    Power,
    Toughness,
    Loyalty,
    Color,
    Types,
    Subtypes,
    Supertypes,
    ConvertedManaCost,
    Controller
};

enum class EffectOperation
{
    Invalid = 0,

    /// <summary>
    /// Set a value, discarding any prior value.
    /// </summary>
    Set,

    /// <summary>
    /// Add to the prior value.
    /// </summary>
    Add,

    /// <summary>
    /// Subtract from the prior value.
    /// </summary>
    Subtract,

    /// <summary>
    /// Multiply the prior value by the layered effect's Modification.
    /// </summary>
    Multiply,

    /// <summary>
    /// Perform a bitwise "or" operation.
    /// </summary>
    BitwiseOr,

    /// <summary>
    /// Perform a bitwise "and" operation.
    /// </summary>
    BitwiseAnd,

    /// <summary>
    /// Perform a bitwise "exclusive or" operation.
    /// </summary>
    BitwiseXor
};

/// <summary>
/// Parameter struct for AddLayeredEffect(...)
/// </summary>
struct LayeredEffectDefinition
{
public:
    /// <summary>
    /// Which attribute this layered effect applies to.
    /// </summary>
    AttributeKey Attribute;

    /// <summary>
    /// What mathematical or bitwise operation this layer performs.
    /// See EffectOperation for details.
    /// </summary>
    EffectOperation Operation;

    /// <summary>
    /// The operand used for this layered effect's Operation.
    /// For example, if Operation is EffectOperation.Add, this is the
    /// amount that is added.
    /// </summary>
    int Modification;

    /// <summary>
    /// Which layer to apply this effect in. Smaller numbered layers
    /// get applied first. Layered effects with the same layer get applied
    /// in the order that they were added. (timestamp order)
    /// </summary>
    int Layer;

private:
    AttributeKey GetAttribute() { return Attribute; };
    EffectOperation GetOperation() { return Operation; };
    int GetModification() { return Modification; };
    int GetLayer() { return Layer; };
};

/// <summary>
/// Any object that implements this interface has a set of "base" attributes
/// that represent the default state of that object. However, the game engine
/// may apply one or more "layered effects" to modify those attributes. An
/// object's "current" attributes are always equal to the base attributes
/// with all layered effects applied, in the proper order. Any change to the
/// base attribute or layered effects should immediately be reflected in the
/// current attribute.
/// </summary>
class ILayeredAttributes
{
public:
    virtual ~ILayeredAttributes() {};

    /// <summary>
    /// Set the base value for an attribute on this object. All base values
    /// default to 0 until set. Note that resetting a base attribute does not
    /// alter any existing layered effects.
    /// </summary>
    /// <param name="key">The attribute being set.</param>
    /// <param name="value">The new base value.</param>
    virtual void SetBaseAttribute(AttributeKey key, int value) = 0;

    /// <summary>
    /// Return the current value for an attribute on this object. Will
    /// be equal to the base value, modified by any applicable layered
    /// effects.
    /// </summary>
    /// <param name="key">The attribute being read.</param>
    /// <returns>The current value of the attribute, accounting for all layered effects.</returns>
    virtual int GetCurrentAttribute(AttributeKey key) = 0;

    /// <summary>
    /// Applies a new layered effect to this object's attributes. See
    /// LayeredEffectDefinition for details on how layered effects are
    /// applied. Note that any number of layered effects may be applied
    /// at any given time. Also note that layered effects are not necessarily
    /// applied in the same order they were added. (see LayeredEffectDefinition.Layer)
    /// </summary>
    /// <param name="effect">The new layered effect to apply.</param>
    virtual void AddLayeredEffect(LayeredEffectDefinition effect) = 0;

    /// <summary>
    /// Removes all layered effects from this object. After this call,
    /// all current attributes will be equal to the base attributes.
    /// </summary>
    virtual void ClearLayeredEffects() = 0;
};