## tables package continue showing Hmisc messages although silencing it
suppressPackageStartupMessages(
library(tables, quietly = T, warn.conflicts = F)
)
library(plyr,   quietly = T, warn.conflicts = F)
library(dplyr,  quietly = T, warn.conflicts = F)
library(tidyr,  quietly = T, warn.conflicts = F)

bt <- booktabs()

dir.create("tables", showWarnings = FALSE)

## Format for the Latex tables.
fmt_tm <- function(x, decimals = 2, ...) {
  x <- round(x * 10^decimals) / 10^decimals
  s <- format(x, ...)
  s <- tables::latexNumeric(s)
  s
}

## Change tabular environment to spread the table to the full column width.
change_tabular_env <- function(file) {
  if (version$os == "linux-gnu") {
    cmd <- paste(paste0("sed -i 's/\\\\begin{tabular}{/\\\\begin{tabular}",
                        "{\\\\columnwidth}{@{\\\\extracolsep{\\\\fill}}/g'"),
                 file)
    system(cmd)
    cmd <- paste("sed -i '1 s/}$/@{}}/'", file)
    system(cmd)
    cmd <- paste("sed -i 's/tabular/tabular\\*/g'", file)
    system(cmd)
  }
  else
    warning(paste("change_tabular_env not implemented for", version$os))
}


## Import the raw data of the experiments
srk_tmp <- read.csv("exp-results.csv", header = T, row.names = NULL)

## Get the instance names and the score generation group for the OP.
srk_tmp$instance <-  vapply(strsplit(as.character(srk_tmp$name), "-"),
                            `[`, 1, FUN.VALUE = character(1))
srk_tmp$gen <-  capitalize(vapply(strsplit(as.character(srk_tmp$name), "-"),
                                  `[`, 2, FUN.VALUE = character(1)))

## Convert the time from nanoseconds to milliseconds
srk_tmp <- srk_tmp %>%
  mutate(pre_time = pre_time / 1000) %>%
  mutate(sep_time = sep_time / 1000) %>%
  mutate(cut_time = cut_time / 1000)

## Group data accordingly
srk_tmp <- srk_tmp %>%
  mutate(srk        = paste(strat, s2, sep = "_")) %>%
  mutate(separation = paste(gomoryhu, s3, extra, sep = "_")) %>%
  mutate(maxinout   = factor(paste(max_in, max_out, sep = "x"))) %>%
  mutate(size       = factor(ifelse(nv <= 1500, "Medium", "Large"),
                             levels = c("Medium", "Large")))

srk_tmp$srk <- revalue(srk_tmp$srk,
                       c("0_0" = "NO",
                         "1_0" = "C1",
                         "2_0" = "C1C2",
                         "3_0" = "C1C2C3",
                         "4_0" = "S1",
                         "4_1" = "S1S2"
                         ))
## Reorder the shrinking strategies and set NO as the reference strategy
srk_tmp <- srk_tmp %>%
  mutate(srk = factor(srk, levels = c("NO", "C1", "C1C2", "C1C2C3",
                                      "S1", "S1S2")))
srk_tmp$srk <-  factor(srk_tmp$srk) %>% relevel(srk, ref = "NO")

## Reorder the separation strategies and set Hong as the reference strategy
srk_tmp$separation <- revalue(srk_tmp$separation,
                              c("0_0_0" = "EH",
                                "0_1_0" = "DH",
                                "0_1_1" = "DHI",
                                "1_0_0" = "EPG"
                                ))
srk_tmp$separation <- factor(srk_tmp$separation) %>% relevel(srk, ref = "EH")

## Reorder the cut generation strategies and set 1x1 as the reference strategy
srk_tmp <- srk_tmp %>%
  mutate(maxinout = factor(maxinout, levels = c("1x1", "10x10")))
srk_tmp$maxinout <- factor(srk_tmp$maxinout) %>% relevel(maxinout, ref = "1x1")

## Clone the results of DH-NO to DHI-NO
dhe_no <- srk_tmp[which(srk_tmp$srk == "NO" & srk_tmp$separation == "DH"), ]
dhe_no$separation <- "DHI"
srk_tmp <- rbind(srk_tmp, dhe_no)

## Calc the size of the shrunk graph in relation to
## the support graph (percentage)
srk_tmp$nv_rel <- (srk_tmp$snv) / srk_tmp$gnv * 100
srk_tmp$na_rel <- (srk_tmp$sna) / srk_tmp$gna * 100

## Group the results by instance, vertex score generation, shrinking strategy,
## separation strategy, and cut generation strategy
srk <- ddply(srk_tmp,
             .(instance,  gen, srk, separation, maxinout), summarize
             , nv              = mean(nv)
             , gnv             = mean(gnv)
             , gna             = mean(gna)
             , snv             = mean(snv)
             , sna             = mean(sna)
             , sep_count_extra = mean(sep_count_extra)
             , pre_count_qsets = mean(pre_count_qsets)
             , sep_count_qsets = mean(sep_count_qsets)
             , nsec            = mean(valid)
             , maxv            = mean(maxval)
             , pre_time        = mean(pre_time)
             , sep_time        = mean(sep_time)
             , cut_time        = mean(cut_time)
             , pre_count_c1    = mean(pre_count_c1)
             , pre_count_c2    = mean(pre_count_c2)
             , pre_count_c3    = mean(pre_count_c3)
             , pre_count_s1    = mean(pre_count_s1)
             , pre_count_s2    = mean(pre_count_s2)
             , pre_count_queue = mean(pre_count_queue)
             , size            = unique(size)
             , nv_rel          = mean(nv_rel)
             , na_rel          = mean(na_rel)
)

## Calc the number of violated cuts per unit  of time (millisecond)
## found by each combination of strategies
srk <- srk %>% mutate(cxms = nsec / cut_time)


###################################################
## Generate the Latex tables                     ##
###################################################

############
## 1. Graph size and separation time in relation to the reference strategy
cat("Building the table of the algorithm speedups..........")
srk$ref_time <- 0
for (instance in unique(srk$instance)) {
  for (gen in unique(srk$gen)) {
    for (shrink in levels(srk$srk)) {
      for (separation in levels(srk$separation)) {
        srk[which(srk$instance == instance & srk$gen == gen &
                  srk$srk == shrink & srk$separation == separation), ]$ref_time <-
          mean(srk[which(srk$instance == instance & srk$gen == gen &
                         srk$srk == "NO" & srk$separation == "EH"), ]$sep_time)
      }
    }
  }
}

srk$time_rel <- srk$ref_time / srk$sep_time

srk_comp_time <- tabular((Size = size) * (Shrinking = srk)  ~
                           (Heading("Preprocess") * 1 * Heading("Graph Size") *
                            Format(fmt_tm(decimals = 2)) * mean *
                            (Heading("$\\%|\\bar{V}|$") * (nv_rel) +
                             Heading("$\\%|\\bar{E}|$") * (na_rel)
                           ) +
                            Heading("Separation") * 1 * Heading("Speedup") *
                            Format(fmt_tm(decimals = 0)) * separation *
                            Heading() * mean * (Heading() * time_rel)
                          ),
                         data = srk
)
#srk_comp_time

rowLabels(srk_comp_time)[7, 1] <- "\\midrule Large"
outf <- paste0("tables/comparison-sep-srk-time.tex")
outt <- latex(srk_comp_time, file = outf)
change_tabular_env(outf)
cat("OK\n")

############
## 2. Time and obtained Q sets by shrinking and separation strategy
cat("Building the table of the times and obtained Q sets...")
srk_comp_cut <- tabular((Size = size) * (Shrinking = srk) ~
                        Format(fmt_tm(decimals = 1)) *
                        (Heading("Preprocess") * 1 *
                         (Heading("All") *
                          (Heading("\\#Q") * mean * Heading() * pre_count_qsets +
                           Heading("Time") * mean * Heading() * pre_time
                          )
                         ) +
                         Heading("Separation") * Heading() * separation *
                         Format(fmt_tm(decimals = 1)) *
                         (Heading("\\#Q") * mean * Heading() * sep_count_qsets +
                          Heading("Time") * mean * Heading() * sep_time
                         )
                        ),
                        data = srk
)
#srk_comp_cut

rowLabels(srk_comp_cut)[7, 1] <- "\\midrule Large"
outf <- paste0("tables/comparison-sep-srk-qsets.tex")
outt <- latex(srk_comp_cut, file = outf)
change_tabular_env(outf)
cat("OK\n")

############
## 3. Tables in the supplementary material
cat("Building the tables in the supplementary material\n")
srk$maxinout <- revalue(srk$maxinout,
                              c("1x1" = "1x1 (10 runs)",
                                "10x10" = "10x10 (10 runs)"
                                ))
for (inst in unique(srk$instance)) {
  cat("  -", inst, paste0(paste0(rep(".", 8 - nchar(inst)), collapse = ""), "..."))
  instdata <- srk %>% filter(instance == as.character(inst))

  # Size
  srk_comp_size <- tabular((Shrinking = srk) ~
                           (Heading() * mean * (`$|V|$` = nv) +
                            Heading() * Factor(gen) *
                            (Heading("Support graph") *
                             (Heading() * mean * (`$|\\bar{V}|$` = gnv) +
                              Heading() * mean * (`$|\\bar{E}|$` = gna)
                             ) +
                             Heading("Shrunk graph", override = T) *
                             (Heading() * mean * (`$|\\bar{V}|$` = snv) +
                              Heading() * mean * (`$|\\bar{E}|$` = sna)
                             ) +
                             Heading("Preprocess") *
                             (Heading() * Format(fmt_tm()) * mean * (`\\#Q` = pre_count_qsets) +
                              Heading() * Format(fmt_tm(decimals = 2)) *
                              mean * (Time = pre_time)
                             )
                            )
                           ),
                           data = instdata
  )
  outf <- paste0("tables/comparison-size-", inst, ".tex")
  outt <- latex(srk_comp_size, file = outf)
  change_tabular_env(outf)

  # Cuts
  srk_comp_cuts <- tabular(((Sep. = separation) * (Shrinking = srk)) ~
                           (Heading() * Factor(gen) *
                            (Heading("Separation") * 1 * Heading("(20 runs)") *
                             (Heading() * mean *
                              Format(fmt_tm(decimals = 1)) * (`\\#Q` = sep_count_qsets) +
                              Heading() * mean *
                              Format(fmt_tm(decimals = 1)) * (Time = sep_time)
                             ) +
                             Heading("SEC Generation") * maxinout *
                             (Heading() * mean * Format(fmt_tm(decimals = 1)) * (`\\#SEC` = nsec) +
                              Heading() * mean * Format(fmt_tm(decimals = 1)) * (Time =  cut_time)
                             )
                            )
                           ),
                           data = instdata
  )

  rowLabels(srk_comp_cuts)[7, 1] <- "\\midrule DH"
  rowLabels(srk_comp_cuts)[13, 1] <- "\\midrule DHI"
  rowLabels(srk_comp_cuts)[19, 1] <- "\\midrule EPG"
  outf <- paste0("tables/comparison-cuts-", inst, ".tex")
  outt <- latex(srk_comp_cuts, file = outf)
  change_tabular_env(outf)

  # Counts
  srk_comp_counts <- tabular((Shrinking = Factor(srk)) ~
                             (Heading() * Factor(gen) *
                              #Format(fmt_tm(decimals = 1)) *
                              (Heading("Preprocess") *
                               (Heading() * mean * Format(fmt_tm()) * (`C1`    = pre_count_c1)   +
                                Heading() * mean * Format(fmt_tm()) * (`C2`    = pre_count_c2)   +
                                Heading() * mean * Format(fmt_tm()) * (`C3`    = pre_count_c3)   +
                                Heading() * mean * Format(fmt_tm()) * (`S1`    = pre_count_s1)   +
                                Heading() * mean * Format(fmt_tm()) * (`S2`    = pre_count_s2)   +
                                Heading() * mean * Format(fmt_tm(decimals = 2)) *(`H`     = pre_count_queue)
                               ) +
                               Heading("DHI") * mean * Format(fmt_tm(decimals = 1)) *(`Extra` = sep_count_extra)
                              )
                             ),
                             data = instdata)

  outf <- paste0("tables/comparison-counts-", inst, ".tex")
  outt <- latex(srk_comp_counts, file = outf)
  change_tabular_env(outf)
  cat("OK\n")
}
