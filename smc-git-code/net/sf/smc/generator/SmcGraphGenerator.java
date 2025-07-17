//
// The contents of this file are subject to the Mozilla Public
// License Version 1.1 (the "License"); you may not use this file
// except in compliance with the License. You may obtain a copy
// of the License at http://www.mozilla.org/MPL/
//
// Software distributed under the License is distributed on an
// "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
// implied. See the License for the specific language governing
// rights and limitations under the License.
//
// The Original Code is State Machine Compiler (SMC).
//
// The Initial Developer of the Original Code is Charles W. Rapp.
// Portions created by Charles W. Rapp are
// Copyright (C) 2005, 2008. Charles W. Rapp.
// All Rights Reserved.
//
// Contributor(s):
//   Eitan Suez contributed examples/Ant.
//   (Name withheld) contributed the C# code generation and
//   examples/C#.
//   Francois Perrad contributed the Python code generation and
//   examples/Python.
//   Chris Liscio contributed the Objective-C code generation
//   and examples/ObjC.
//
// RCS ID
// $Id$
//
// CHANGE LOG
// (See the bottom of this file.)
//

package net.sf.smc.generator;

import java.io.ByteArrayOutputStream;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import net.sf.smc.model.SmcAction;
import net.sf.smc.model.SmcElement;
import net.sf.smc.model.SmcElement.TransType;
import net.sf.smc.model.SmcFSM;
import net.sf.smc.model.SmcGuard;
import net.sf.smc.model.SmcMap;
import net.sf.smc.model.SmcParameter;
import net.sf.smc.model.SmcState;
import net.sf.smc.model.SmcTransition;
import net.sf.smc.model.SmcVisitor;

/**
 * Visits the abstract syntax tree, emitting a Graphviz diagram.
 * @see SmcElement
 * @see SmcCodeGenerator
 * @see SmcVisitor
 * @see SmcOptions
 *
 * @author <a href="mailto:rapp@acm.org">Charles Rapp</a>
 */

public final class SmcGraphGenerator
    extends SmcCodeGenerator
{
//---------------------------------------------------------------
// Member methods
//

    //-----------------------------------------------------------
    // Constructors.
    //

    /**
     * Creates a GraphViz code generator for the given options.
     * @param options The target code generator options.
     */
    public SmcGraphGenerator(final SmcOptions options)
    {
        super (options, "dot");

        _transitions =
            new ByteArrayOutputStream(TRANSITION_OUTPUT_SIZE);
        _transitionSource = new PrintStream(_transitions);
    } // end of SmcGraphGenerator(SmcOptions)

    //
    // end of Constructors.
    //-----------------------------------------------------------

    //-----------------------------------------------------------
    // SmcVisitor Abstract Method Impelementation.
    //

    /**
     * Emits GraphViz code for the finite state machine.
     * @param fsm emit GraphViz code for this finite state machine.
     */
    public void visit(final SmcFSM fsm)
    {
        final PrintStream source = getStream(fsm);
        final Map<String, String> pushEntryMap =
            new HashMap<String, String>();
        final Map<String, String> popTransMap =
            new HashMap<String, String>();
        final Map<String, String> pushStateMap =
            new HashMap<String, String>();

        // Create one overall graph and place each map in a
        // subgraph.
        source.print("digraph ");
        source.print(_srcfileBase);
        source.println(" {");
        source.println();
        source.println("    node");
        source.println("        [shape=Mrecord width=1.5];");
        source.println();

        // Have each map generate its subgraph.
        for (SmcMap map: fsm.getMaps())
        {
            String mapName = map.getName();

            source.print("    subgraph cluster_");
            source.print(mapName);
            source.println(" {");
            source.println();
            source.print("        label=\"");
            source.print(mapName);
            source.println("\";");
            source.println();

            map.accept(this);

            // Output the subgraph's closing brace.
            source.println("    }");
            source.println();
        }

        // Output the transitions *outside* of the subgraphs.
        source.println("    //");
        source.println("    // Transitions (Edges)");
        source.println("    //");
        source.println(_transitions.toString());

        // Output the digraph's closing brace.
        source.println("}");

        return;
    } // end of visit(SmcFSM)

    /**
     * Emits GraphViz code for the FSM map.
     * @param map emit GraphViz code for this map.
     */
    public void visit(final SmcMap map)
    {
        final PrintStream source = getStream(map);
        final String mapName = map.getName();
        final SmcState defaultState = map.getDefaultState();
        final String startStateName = map.getFSM().getStartState();
        final Map<String, String> pushEntryMap = new HashMap<String, String>();
        final Map<String, String> popTransMap = new HashMap<String, String>();
        final Map<String, String> pushStateMap = new HashMap<String, String>();
        boolean needEnd = false;
        String startMapName =
            startStateName.substring(0, startStateName.indexOf(":"));

        source.println("        //");
        source.println("        // States (Nodes)");
        source.println("        //");
        source.println();

        // Output the state names first.
        for (SmcState state: map.getStates())
        {
            state.accept(this);
        }

        for (SmcState state: map.getStates())
        {
            for (SmcTransition transition: state.getTransitions())
            {
                for (SmcGuard guard: transition.getGuards())
                {
                    String endStateName = guard.getEndState();
                    TransType transType = guard.getTransType();

                    if (transType == TransType.TRANS_PUSH)
                    {
                        String pushStateName = guard.getPushState();
                        String pushMapName;
                        int index;

                        if (endStateName.equals(SmcElement.NIL_STATE) == true)
                        {
                             endStateName = state.getInstanceName();
                        }

                        if ((index = pushStateName.indexOf("::")) >= 0)
                        {
                            pushMapName = pushStateName.substring(0, index);
                        }
                        else
                        {
                            pushMapName = mapName;
                        }

                        pushStateMap.put(mapName + "::" +
                                         endStateName + "::" +
                                         pushMapName, pushMapName);
                    }
                    else if (transType == TransType.TRANS_POP)
                    {
                        String popKey = endStateName;
                        String popVal = endStateName;
                        String popArgs;

                        if (_graphLevel == GRAPH_LEVEL_2 &&
                            (popArgs = guard.getPopArgs()) != null &&
                            popArgs.length() > 0)
                        {
                            popKey += ", ";
                            popVal += ", ";
                            popKey += _escape(_normalize(popArgs));
                            // If the argument contains line separators,
                            // then replace them with a "\n" so Graphviz knows
                            // about the line separation.
                            popVal += _escape(popArgs).replaceAll(
                                "\\n", "\\\\\\l");
                        }
                        popTransMap.put(popKey, popVal);
                        needEnd = true;
                    }
                }
            }

            if (defaultState != null)
            {
                for (SmcTransition transition: defaultState.getTransitions())
                {
                    String transName = transition.getName();

                    if (state.callDefault(transName) == true)
                    {
                        for (SmcGuard guard: transition.getGuards())
                        {
                            if (state.findGuard(
                                    transName, guard.getCondition()) == null)
                            {
                                String endStateName = guard.getEndState();
                                TransType transType = guard.getTransType();

                                if (transType == TransType.TRANS_PUSH)
                                {
                                    String pushStateName = guard.getPushState();
                                    String pushMapName;
                                    int index;

                                    if (endStateName.equals(SmcElement.NIL_STATE) == true)
                                    {
                                        endStateName = state.getInstanceName();
                                    }

                                    if ((index = pushStateName.indexOf("::")) >= 0)
                                    {
                                        pushMapName = pushStateName.substring(0, index);
                                    }
                                    else
                                    {
                                        pushMapName = mapName;
                                    }

                                    pushStateMap.put(mapName + "::" + endStateName + "::" + pushMapName, pushMapName);
                                }
                                else if (transType == TransType.TRANS_POP)
                                {
                                    String popKey = endStateName;
                                    String popVal = endStateName;
                                    String popArgs;

                                    if (_graphLevel == GRAPH_LEVEL_2 &&
                                        (popArgs = guard.getPopArgs()) != null &&
                                         popArgs.length() > 0)
                                    {
                                        popKey += ", ";
                                        popVal += ", ";
                                        popKey += _escape(_normalize(popArgs));
                                        // If the argument contains line separators,
                                        // then replace them with a "\n" so Graphviz knows
                                        // about the line separation.
                                        popVal += _escape(popArgs).replaceAll(
                                            "\\n", "\\\\\\l");
                                    }
                                    popTransMap.put(popKey, popVal);
                                    needEnd = true;
                                }
                            }
                        }
                    }
                }
            }
        }

        // Now output the pop transitions as "nodes".
        for (String pname: popTransMap.keySet())
        {
            source.print("        \"");
            source.print(mapName);
            source.print("::pop(");
            source.print(pname);
            source.println(")\"");
            source.println("            [label=\"\" width=1]");
            source.println();
        }

        if (needEnd == true)
        {
            // Output the end node
            source.print("        \"");
            source.print(mapName);
            source.println("::%end\"");
            source.println(
                "            [label=\"\" shape=doublecircle style=filled fillcolor=black width=0.15];");
            source.println();
        }

        // Now output the push composite state.
        for (String pname: pushStateMap.keySet())
        {
            source.print("        \"");
            source.print(pname);
            source.println("\"");
            source.print("            [label=\"{");
            source.print(pushStateMap.get(pname));
            source.println("|O-O\\r}\"]");
            source.println();
        }

        if (startMapName.equals(mapName) == true)
        {
            // Output the start node only in the right map
            source.println("        \"%start\"");
            source.println("            [label=\"\" shape=circle style=filled fillcolor=black width=0.25];");
            source.println();
        }

        // Now output the push actions as "nodes".
        for (SmcMap map2: map.getFSM().getMaps())
        {
            for (SmcState state: map2.getAllStates())
            {
                for (SmcTransition transition: state.getTransitions())
                {
                    for (SmcGuard guard: transition.getGuards())
                    {
                        if (guard.getTransType() == TransType.TRANS_PUSH)
                        {
                            String pushStateName = guard.getPushState();

                            if (pushStateName.indexOf(mapName) == 0)
                            {
                                pushEntryMap.put(pushStateName, "");
                            }
                        }
                    }
                }
            }
        }

        for (String pname: pushEntryMap.keySet())
        {
            // Output the push action.
            source.print("        \"push(");
            source.print(pname);
            source.println(")\"");
            source.println("            [label=\"\" shape=plaintext];");
            source.println();
        }

        // For each state, output its transitions.
        for (SmcState state: map.getStates())
        {
            _state = state;
            for (SmcTransition transition:
                     state.getTransitions())
            {
                transition.accept(this);
            }

            if (defaultState != null)
            {
                for (SmcTransition transition: defaultState.getTransitions())
                {
                    String transName = transition.getName();
                    if (state.callDefault(transName) == true)
                    {
                        for (SmcGuard guard: transition.getGuards())
                        {
                            if (state.findGuard(
                                    transName, guard.getCondition()) == null)
                            {
                                guard.accept(this);
                            }
                        }
                    }
                }
            }
        }

        // Now output the pop transitions.
        for (String pname: popTransMap.keySet())
        {
            _transitionSource.println();
            _transitionSource.print("    \"");
            _transitionSource.print(mapName);
            _transitionSource.print("::pop(");
            _transitionSource.print(pname);
            _transitionSource.print(")\" -> \"");
            _transitionSource.print(mapName);
            _transitionSource.println("::%end\"");
            _transitionSource.print("        [label=\"pop(");
            _transitionSource.print(popTransMap.get(pname));
            _transitionSource.println(");\\l\"];");
        }

        // Now output the composite state transition.
        for (String pname: pushStateMap.keySet())
        {
            _transitionSource.println();
            _transitionSource.print("    \"");
            _transitionSource.print(pname);
            _transitionSource.print("\" -> \"");
            _transitionSource.print(pname.substring(0, pname.lastIndexOf("::")));
            _transitionSource.println("\"");
            _transitionSource.println("        [label=\"pop/\"]");
        }

        if (startMapName.equals(mapName) == true)
        {
            // Output the start transition only in the right map
            _transitionSource.println();
            _transitionSource.print("    \"%start\" -> \"");
            _transitionSource.print(startStateName);
            _transitionSource.println("\"");
        }

        // Now output the push actions as entry "transition".
        for (String pname: pushEntryMap.keySet())
        {
            _transitionSource.println();
            _transitionSource.print("    \"push(");
            _transitionSource.print(pname);
            _transitionSource.print(")\" -> \"");
            _transitionSource.print(pname);
            _transitionSource.println("\"");
            _transitionSource.println("        [arrowtail=odot];");
        }

        return;
    } // end of visit(SmcMap)

    /**
     * Emits GraphViz code for this FSM state.
     * @param state emits GraphViz code for this state.
     */
    public void visit(final SmcState state)
    {
        final PrintStream source = getStream(state);
        final SmcMap map = state.getMap();
        final SmcState defaultState = map.getDefaultState();
        final String mapName = map.getName();
        final String instanceName = state.getInstanceName();

        // The state name must be fully-qualified because
        // Graphviz does not allow subgraphs to share node names.
        // Place the node name in quotes.
        source.print("        \"");
        source.print(mapName);
        source.print("::");
        source.print(instanceName);
        source.println("\"");

        source.print("            [label=\"{");
        source.print(instanceName);

        // For graph level 1 & 2, output entry and exit actions.
        if (_graphLevel >= GRAPH_LEVEL_1)
        {
            List<SmcAction> actions;
            Iterator<SmcAction> it;
            boolean empty = true;

            actions = state.getEntryActions();
            if (actions == null && defaultState != null)
            {
                actions = defaultState.getEntryActions();
            }
            if (actions != null)
            {
                if (empty == true)
                {
                    source.print("|");
                    empty = false;
                }
                source.print("Entry/\\l");

                // Output the entry actions, one per line.
                for (SmcAction action: actions)
                {
                    source.print(INDENT_ACTION);
                    action.accept(this);
                }
            }

            actions = state.getExitActions();
            if (actions == null && defaultState != null)
            {
                actions = defaultState.getExitActions();
            }
            if (actions != null)
            {
                if (empty == true)
                {
                    source.print("|");
                    empty = false;
                }
                source.print("Exit/\\l");

                // Output the exit actions, one per line.
                for (SmcAction action: actions)
                {
                    source.print(INDENT_ACTION);
                    action.accept(this);
                }
            }

            // Starts a new compartment for internal events
            empty = true;
            for (SmcTransition transition: state.getTransitions())
            {
                for (SmcGuard guard: transition.getGuards())
                {
                    String endStateName = guard.getEndState();
                    TransType transType = guard.getTransType();

                    if (isLoopback(transType, endStateName) &&
                        transType != TransType.TRANS_PUSH)
                    {
                        String transName = transition.getName();
                        String condition = guard.getCondition();
                        String pushStateName = guard.getPushState();
                        actions = guard.getActions();

                        if (empty == true)
                        {
                            source.print("|");
                            empty = false;
                        }
                        source.print(transName);

                        // Graph Level 2: Output the transition parameters.
                        if (_graphLevel == GRAPH_LEVEL_2)
                        {
                            final List<SmcParameter> parameters =
                                transition.getParameters();
                            final Iterator<SmcParameter> pit =
                                parameters.iterator();
                            SmcParameter param;
                            String sep;

                            source.print("(");
                            for (sep = ""; pit.hasNext() == true; sep = ", ")
                            {
                                source.print(sep);

                                // Make sure the parameter is output in the
                                // subgraph.
                                param = pit.next();
                                param.setInSubgraph(true);
                                param.accept(this);
                            }
                            source.print(")");
                        }

                        // Output the guard.
                        if (condition != null && condition.length() > 0)
                        {
                            String tmp = _escape(condition);

                            // If the condition contains line separators,
                            // then replace them with a "\n" so Graphviz knows
                            // about the line separation.
                            tmp = tmp.replaceAll("\\n", "\\\\\\l");

                            // Not needed when label in edge !!
                            tmp = tmp.replaceAll(">", "\\\\>");
                            tmp = tmp.replaceAll("<", "\\\\<");
                            tmp = tmp.replaceAll("\\|", "\\\\|");

                            source.print("\\l\\[");
                            source.print(tmp);
                            source.print("\\]");
                        }

                        source.print("/\\l");

                        if (actions != null)
                        {
                            // Output the actions, one per line.
                            for (SmcAction action: actions)
                            {
                                source.print(INDENT_ACTION);
                                action.accept(this);
                            }
                        }

                        if (transType == TransType.TRANS_PUSH)
                        {
                            source.print(INDENT_ACTION);
                            source.print("push(");
                            source.print(pushStateName);
                            source.print(")\\l");
                        }
                    }
                }
            }

            if (defaultState != null)
            {
                for (SmcTransition transition: defaultState.getTransitions())
                {
                    String transName = transition.getName();
                    if (state.callDefault(transName) == true)
                    {
                        for (SmcGuard guard: transition.getGuards())
                        {
                            if (state.findGuard(transName, guard.getCondition()) == null)
                            {
                                String endStateName = guard.getEndState();
                                TransType transType = guard.getTransType();

                                if (isLoopback(transType, endStateName) &&
                                    transType != TransType.TRANS_PUSH)
                                {
                                    transName = transition.getName();
                                    String condition = guard.getCondition();
                                    String pushStateName = guard.getPushState();
                                    actions = guard.getActions();

                                    if (empty == true)
                                    {
                                        source.print("|");
                                        empty = false;
                                    }
                                    source.print(transName);

                                    // Graph Level 2: Output the transition parameters.
                                    if (_graphLevel == GRAPH_LEVEL_2)
                                    {
                                        final List<SmcParameter> parameters =
                                            transition.getParameters();
                                        final Iterator<SmcParameter> pit =
                                            parameters.iterator();
                                        SmcParameter param;
                                        String sep;

                                        source.print("(");
                                        for (sep = ""; pit.hasNext() == true; sep = ", ")
                                        {
                                            source.print(sep);

                                            // Make sure the parameter is output in the
                                            // subgraph.
                                            param = pit.next();
                                            param.setInSubgraph(true);
                                            param.accept(this);
                                        }
                                        source.print(")");
                                    }

                                    // Output the guard.
                                    if (condition != null && condition.length() > 0)
                                    {
                                        String tmp = _escape(condition);

                                        // If the condition contains line separators,
                                        // then replace them with a "\n" so Graphviz knows
                                        // about the line separation.
                                        tmp = tmp.replaceAll("\\n", "\\\\\\l");

                                        // Not needed when label in edge !!
                                        tmp = tmp.replaceAll(">", "\\\\>");
                                        tmp = tmp.replaceAll("<", "\\\\<");
                                        tmp = tmp.replaceAll("\\|", "\\\\|");

                                        source.print("\\l\\[");
                                        source.print(tmp);
                                        source.print("\\]");
                                    }

                                    source.print("/\\l");

                                    if (actions != null)
                                    {
                                        // Output the actions, one per line.
                                        for (SmcAction action: actions)
                                        {
                                            source.print(INDENT_ACTION);
                                            action.accept(this);
                                        }
                                    }

                                    if (transType == TransType.TRANS_PUSH)
                                    {
                                        source.print(INDENT_ACTION);
                                        source.print("push(");
                                        source.print(pushStateName);
                                        source.print(")\\l");
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        source.println("}\"];");
        source.println();

        return;
    } // end of visit(SmcState)

    /**
     * Emits GraphViz code for this FSM transition.
     * @param transition emits GraphViz code for this transition.
     */
    public void visit(final SmcTransition transition)
    {
        for (SmcGuard guard: transition.getGuards())
        {
            guard.accept(this);
        }

        return;
    } // end of visit(SmcTransition)

    /**
     * Emits GraphViz code for this FSM transition guard.
     * @param guard emits GraphViz code for this transition guard.
     */
    public void visit(final SmcGuard guard)
    {
        final PrintStream source = getStream(guard);
        final SmcTransition transition = guard.getTransition();
        final SmcMap map = _state.getMap();
        final String mapName = map.getName();
        final String stateName = _state.getInstanceName();
        final String transName = transition.getName();
        final TransType transType = guard.getTransType();
        String endStateName = guard.getEndState();
        final String pushStateName = guard.getPushState();
        final String condition = guard.getCondition();
        final List<SmcAction> actions = guard.getActions();

        // Loopback are added in the state
        if (isLoopback(transType, endStateName) &&
            transType != TransType.TRANS_PUSH)
        {
            return;
        }

        source.println();
        source.print("    \"");
        source.print(mapName);
        source.print("::");
        source.print(stateName);
        source.print("\" -> ");

        if (transType != TransType.TRANS_POP)
        {
            if (endStateName.equals(SmcElement.NIL_STATE) == true)
            {
                endStateName = stateName;
            }

            if (endStateName.indexOf("::") < 0)
            {
                endStateName = mapName + "::" + endStateName;
            }

            source.print("\"");
            source.print(endStateName);

            if (transType == TransType.TRANS_PUSH)
            {
                int index = pushStateName.indexOf("::");

                source.print("::");

                if (index < 0)
                {
                    source.print(mapName);
                }
                else
                {
                    source.print(pushStateName.substring(0, pushStateName.indexOf("::")));
                }
            }

            source.println("\"");
        }
        else
        {
            String popArgs = guard.getPopArgs();

            source.print("\"");
            source.print(mapName);
            source.print("::pop(");
            source.print(endStateName);

            if (_graphLevel == GRAPH_LEVEL_2 &&
                popArgs != null &&
                popArgs.length() > 0)
            {
                source.print(", ");
                source.print(_escape(_normalize(popArgs)));
            }

            source.println(")\"");
        }

        source.print("        [label=\"");
        source.print(transName);

        // Graph Level 2: Output the transition parameters.
        if (_graphLevel == GRAPH_LEVEL_2)
        {
            final List<SmcParameter> parameters =
                transition.getParameters();
            final Iterator<SmcParameter> pit =
                parameters.iterator();
            SmcParameter param;
            String sep;

            source.print("(");
            for (sep = ""; pit.hasNext() == true; sep = ", ")
            {
                source.print(sep);

                // Make sure the parameter is output in the same
                // area as the guard.
                param = pit.next();
                param.setInSubgraph(guard.isInSubgraph());
                param.accept(this);
            }
            source.print(")");
        }

        // Graph Level 1, 2: Output the guard.
        if (_graphLevel > GRAPH_LEVEL_0 &&
            condition != null &&
            condition.length() > 0)
        {
            String continueLine = "\\\\";

            source.print("\\l\\[");

            // If the condition contains line separators,
            // then replace them with a "\n" so Graphviz knows
            // about the line separation.
            // 4.3.0: First escape the condition then replace the
            //        line separators.
            source.print(
                _escape(condition).replaceAll(
                    "\\n", "\\\\\\l"));

            source.print("\\]");
        }
        source.print("/\\l");

        // Graph Level 1, 2: output actions.
        if (_graphLevel > GRAPH_LEVEL_0 &&
            actions != null)
        {
            for (SmcAction action: actions)
            {
                action.accept(this);
            }
        }

        if (transType == TransType.TRANS_PUSH)
        {
            source.print("push(");
            source.print(pushStateName);
            source.print(")\\l");
        }

        source.println("\"];");

        return;
    } // end of visit(SmcGuard)

    /**
     * Emits GraphViz code for this FSM action.
     * @param action emits GraphViz code for this action.
     */
    public void visit(final SmcAction action)
    {
        // Entry, Exit actions AND inner loopback transition
        // actions placed into the subgraph. All other actions
        // are in the transition source. Inner loop transition
        // actions are marked as Entry/Exit actions for this
        // reason.
        final PrintStream source = getStream(action);

        // Actions are only reported for graph levels 1 and 2.
        // Graph level 1: only the action name, no arguments.
        // Graph level 2: action name and arguments.
        //
        // Note: do not output an end-of-line.
        if (_graphLevel >= GRAPH_LEVEL_1)
        {
            source.print(action.getName());

            if (_graphLevel == GRAPH_LEVEL_2)
            {
                List<String> arguments = action.getArguments();
                String arg;

                if (action.isProperty() == true)
                {
                    source.print(" = ");

                    arg = arguments.get(0).trim();

                    // If the argument is a quoted string, then
                    // the quotes must be escaped.
                    // First, replace all backslashes with two
                    // backslashes.
                    arg = arg.replaceAll("\\\\", "\\\\\\\\");

                    // Then replace all double quotes with
                    // a backslash double qoute.
                    source.print(
                        arg.replaceAll("\"", "\\\\\""));
                }
                else
                {
                    Iterator<String> it;
                    String sep;

                    source.print("(");

                    // Now output the arguments.
                    for (it = arguments.iterator(),
                             sep = "";
                         it.hasNext() == true;
                         sep = ", ")
                    {
                        arg = (it.next()).trim();

                        source.print(sep);

                        // If the argument is a quoted string, then
                        // the quotes must be escaped.
                        // First, replace all backslashes with two
                        // backslashes.
                        arg = arg.replaceAll("\\\\", "\\\\\\\\");

                        // Then replace all double quotes with
                        // a backslash double qoute.
                        source.print(
                            arg.replaceAll("\"", "\\\\\""));
                    }

                    source.print(")");
                }
            }

            source.print(";\\l");
        }

        return;
    } // end of visit(SmcAction)

    /**
     * Emits GraphViz code for this transition parameter.
     * @param parameter emits GraphViz code for this transition parameter.
     */
    public void visit(final SmcParameter parameter)
    {
        // Entry, Exit actions AND inner loopback transition
        // actions placed into the subgraph. All other actions
        // are in the transition source. Inner loop transition
        // actions are marked as Entry/Exit actions for this
        // reason.
        final PrintStream source = getStream(parameter);

        // Graph Level 2
        source.print(parameter.getName());
        if (parameter.getType().equals("") == false)
        {
            source.print(": ");
            source.print(parameter.getType());
        }

        return;
    } // end of visit(SmcParameter)

    /**
     * Returns the {@code PrintStream} associated with
     * {@code element} based on whether
     * {@link SmcElement#isInSubgraph} returns {@code true}
     * (then return {@code #_source}) or returns {@code false}
     * (then return {@code #__transitionSource}).
     */
    private PrintStream getStream(final SmcElement element)
    {
        return (element.isInSubgraph() == true ?
                _source :
                _transitionSource);
    } // end of getStream(SmcElement)

    // Place a backslash escape character in front of backslashes
    // and doublequotes.
    private static String _escape(String s)
    {
        String retval;

        if (s.indexOf('\\') < 0 && s.indexOf('"') < 0)
        {
            retval = s;
        }
        else
        {
            StringBuffer buffer =
                new StringBuffer(s.length() * 2);
            int index;
            int length = s.length();
            char c;

            for (index = 0; index < length; ++index)
            {
                c = s.charAt(index);
                if (c == '\\' || c == '"')
                {
                    buffer.append('\\');
                }

                buffer.append(c);
            }

            retval = buffer.toString();
        }

        return (retval);
    }

    private static String _normalize(String s)
    {
        int index;
        int length = s.length();
        char c;
        boolean space = false;
        StringBuffer buffer =
            new StringBuffer(length);

        for (index = 0; index < length; ++index)
        {
            c = s.charAt(index);
            if (space)
            {
                if (c != ' ' && c != '\t' && c != '\n')
                {
                    buffer.append(c);
                    space = false;
                }
            }
            else
            {
                if (c == ' ' || c == '\t' || c == '\n')
                {
                    buffer.append(' ');
                    space = true;
                }
                else
                {
                    buffer.append(c);
                }
            }
        }

        return (buffer.toString().trim());
    }

    // Outputs a list of warning and error messages.
//---------------------------------------------------------------
// Member data
//

    /**
     * The currently accepted FSM state.
     */
    private SmcState _state;

    /**
     * Contains the transitions output which is posted outside
     * of the subgraphs <em>but</em> generated when outputing
     * the subgaphs.
     */
    private final ByteArrayOutputStream _transitions;

    /**
     * Encapsulates {@link #_transitions} for easier transition
     * output.
     */
    private final PrintStream _transitionSource;

    //-----------------------------------------------------------
    // Constants.
    //

    /**
     * Indent, DOT notation.
     */
    private static final String INDENT_ACTION =
        "&nbsp;&nbsp;&nbsp;";

    /**
     * Set the transition output stream size to {@value} bytes
     * initially.
     */
    private static final int TRANSITION_OUTPUT_SIZE = 8192;
} // end of class SmcGraphGenerator

//
// CHANGE LOG
// $Log$
// Revision 1.9  2010/05/27 10:15:36  fperrad
// fix #3007678
//
// Revision 1.8  2010/03/08 17:02:40  fperrad
// New representation of the Default state. The result is full UML.
//
// Revision 1.7  2010/03/05 21:29:53  fperrad
// Allows property with Groovy, Lua, Perl, Python, Ruby & Scala
//
// Revision 1.6  2010/03/03 19:18:40  fperrad
// fix property with Graph & Table
//
// Revision 1.5  2009/11/25 22:30:19  cwrapp
// Fixed problem between %fsmclass and sm file names.
//
// Revision 1.4  2009/11/24 20:42:39  cwrapp
// v. 6.0.1 update
//
// Revision 1.3  2009/09/05 15:39:20  cwrapp
// Checking in fixes for 1944542, 1983929, 2731415, 2803547 and feature 2797126.
//
// Revision 1.2  2009/03/27 09:41:47  cwrapp
// Added F. Perrad changes back in.
//
// Revision 1.1  2009/03/01 18:20:42  cwrapp
// Preliminary v. 6.0.0 commit.
//
//
