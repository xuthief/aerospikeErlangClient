%%% Copyright (C) 2013  by Aerospike.  All rights reserved.  
%% -------------------------------------------------------------------------
%% Erlang Application Hello World Example
%% -------------------------------------------------------------------------
%% Simple steps in the Hello World Application
%% (1) Connect to Aerospike
%% (2) Write (put) a "hello world" record in Aerospike
%% (3) Read (get) a "hello world" record from Aerospike
%% (4) Remove (delete) the "hello world" record from Aerospike
%% (5) Disconnect (shutdown) 
%%

%% Define this module:  helloWorld.erl
-module(lsetHelloWorld).

%% Import the Aerospike API module and Define those functions from it that 
%% we will use in this module
-import(aerospike,
   [connect/2, lset_add/7, getAll/5, delete/4, shutdownAll/0 ]).

%% Expose the following to be called either by other modules or by the
%% client shell (command line).
-export([ hello/0 ]).
    
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% hello: Run our sample program
%% %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%% ------------------------------------------------------------------------
hello() ->
  io:format("Hello From the World of Erlang ~n"),
  %% Set the parms for attaching to a local Aerospike Server
  Host = "10.37.129.7",  %% Assume a server is installed and running locally
                       %% ==>  /etc/init.d/citrusleaf start
  Port = 3000,
  NS = "test",
  Set = "test_set",
  {ok, C } = aerospike:connect( Host, Port ),
  Key = "lsetHello4",
    Ldt = "lset",
  MyRecord1 = "Hello World1",
  MyRecord2 = "",
  WriteOps = 0,

% % Write to the Aerospike Server
% AddResults1 = aerospike:lsetAdd( C, NS, Set, Key, Ldt, MyRecord1, WriteOps),
% io:format("Add Results(~p) ~n", [AddResults1]),

    AddResults0 = aerospike:lsetAdd( C, NS, Set, Key, Ldt, <<"aaaaa">>, WriteOps),
  io:format("Add Results(~p) ~n", [AddResults0]),

% % Write to the Aerospike Server
% AddResults2 = aerospike:lsetAdd( C, NS, Set, Key, Ldt, MyRecord2, WriteOps),
% io:format("Add Results(~p) ~n", [AddResults2]),

% % Read from the Aerospike Server
% GetResults2 = aerospike:lsetGetAll( C, NS, Set, Key, Ldt, 0),
% io:format("Get Results(~p) ~n", [GetResults2]),

% % Delete from the Aerospike Server
% aerospike:delete( C, NS, Set, Key, 0 ),

% % remove to the Aerospike Server
% AddResults3 = aerospike:lsetDelete( C, NS, Set, Key, Ldt, MyRecord2, WriteOps),
% io:format("Add Results(~p) ~n", [AddResults3]),

% % Read from the Aerospike Server
% GetResults3 = aerospike:lsetGetAll( C, NS, Set, Key, Ldt, 0),
% io:format("Get Results(~p) ~n", [GetResults3]),

% % remove to the Aerospike Server
% AddResults4 = aerospike:lsetDelete( C, NS, Set, Key, Ldt, MyRecord1, WriteOps),
% io:format("Add Results(~p) ~n", [AddResults4]),

% % Read from the Aerospike Server
% GetResults4 = aerospike:lsetGetAll( C, NS, Set, Key, Ldt, 0),
% io:format("Get Results(~p) ~n", [GetResults4]),

    addList( C, NS, Set, Key, Ldt, MyRecord1, WriteOps, 0),
    io:format("AddList Done ~n"),

%   addList( C, NS, Set, Key, Ldt, MyRecord1, WriteOps, 0),
%   io:format("AddList Done 2 ~n"),

    delList( C, NS, Set, Key, Ldt, MyRecord1, WriteOps, 0),
    io:format("DelList Done 2 ~n"),

  aerospike:shutdownAll(),
  io:format("Goodbye From the World of Erlang ~n"),
  ok. %% Leave on a high note.

-define (Count, 1000).
-define (PrintTimes, 5).

addList( C, NS, Set, Key, Ldt, MyRecord2, WriteOps, Index) ->
     MyRecord = Index,
    % MyRecord = MyRecord2 ++ "2",
    %% MyRecord = "HelloWorld" ++ erlang:integer_to_list(Index),
    if 
        Index < ?Count ->
            printList( C, NS, Set, Key, Ldt, Index),
            % Write to the Aerospike Server
            case aerospike:lsetAdd( C, NS, Set, Key, Ldt, MyRecord, WriteOps) of
                ok ->
                    % io:format("add[~p] error[~p] for ~p~n", [MyRecord, ok, Index]),
                    done;
                Error ->
                    io:format("add[~p] error[~p] for ~p~n", [MyRecord, Error, Index]),
                    Error
            end,
            addList( C, NS, Set, Key, Ldt, MyRecord, WriteOps, Index+1);
        true ->
            io:format("done add list ~n")
    end.


delList( C, NS, Set, Key, Ldt, MyRecord2, WriteOps, Index) ->
     MyRecord = Index,
    % MyRecord = MyRecord2 ++ "2",
    % MyRecord = "HelloWorld" ++ erlang:integer_to_list(Index),
    if 
        Index < ?Count ->
            printList( C, NS, Set, Key, Ldt, Index),
            % Write to the Aerospike Server
            case aerospike:lsetDelete( C, NS, Set, Key, Ldt, MyRecord, WriteOps) of
                ok ->
                    done;
                Error ->
                    io:format("del[~p] error[~p] for ~p~n", [MyRecord, Error, Index]),
                    Error
            end,
            delList( C, NS, Set, Key, Ldt, MyRecord, WriteOps, Index+1);
        true ->
            io:format("done del list ~n")
    end.

printList( C, NS, Set, Key, Ldt, Index) ->
    PrintInterval = ?Count / ?PrintTimes,
    Rem = Index rem round(PrintInterval),
    case Rem of
        0 ->
            GetResults2 = aerospike:lsetGetAll( C, NS, Set, Key, Ldt, 0),
            case GetResults2 of
                {_, _} ->
                    io:format("Get Results Error (~p) ~n", [GetResults2]);
                _ ->
                    io:format("Get Results Count(~p) for Index(~p)~n", [erlang:length(GetResults2), Index])
            end;
            %io:format("Get Results(~p) ~n", [GetResults2]);
        _ ->
            ok
    end.


%% ------------------------------------------------------------------------
%% <EOF> <EOF> <EOF> <EOF> <EOF> <EOF> <EOF> <EOF> <EOF> <EOF> <EOF> <EOF>
%% ------------------------------------------------------------------------
