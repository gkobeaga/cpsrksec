#!/bin/bash

NUM_REP=10
MAX_VALUES="1 10"

if [ -z "$CPSRKSEC_DIR" ]
then
  CPSRKSEC_DIR=./..
fi

EXP_EXEC=$CPSRKSEC_DIR/build/exp/exp
INSTANCES=$CPSRKSEC_DIR/exp/instances/*.graph

if [ 1 -eq 1 ]
then
for INSTANCE in $INSTANCES
do
  for RULE in  $(seq 0 4)
  do
    if [ "$RULE" -eq 4 ]
    then
      TRY_SEC2=1
    else
      TRY_SEC2=0
    fi
    for SEC2 in $(seq 0 $TRY_SEC2)
    do
      for SEC3 in $(seq 0 1)
      do
        if [ "$RULE" -eq 0 ] || [ "$SEC3" -eq 0 ]
        then
          TRY_EXTRA=0
        else
          TRY_EXTRA=1
        fi
        for EXTRA in $(seq 0 $TRY_EXTRA)
        do
          for MAX in $MAX_VALUES
          do
            for REP in $(seq 1 $NUM_REP)
            do
              ARGS="$RULE $SEC2 $SEC3 $EXTRA $MAX"
              echo "EH $RULE $SEC2 $SEC3 $EXTRA $MAX $INSTANCE"
              $EXP_EXEC --hong $ARGS $INSTANCE
            done
          done
        done
      done
    done
  done
done
fi

for INSTANCE in $INSTANCES
do
  for RULE in  $(seq 0 4)
  do
    if [ "$RULE" -eq 4 ]
    then
      TRY_SEC2=1
    else
      TRY_SEC2=0
    fi
    for SEC2 in $(seq 0 $TRY_SEC2)
    do
      for MAX in $MAX_VALUES
      do
        for REP in $(seq 1 $NUM_REP)
        do
          ARGS="$RULE $SEC2 $MAX"
          echo "EPG $RULE $SEC2 $MAX $INSTANCE"
          $EXP_EXEC --gomoryhu $ARGS $INSTANCE
        done
      done
    done
  done
done
