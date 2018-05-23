import com.atlassian.bamboo.specs.api.BambooSpec;
import com.atlassian.bamboo.specs.api.builders.AtlassianModule;
import com.atlassian.bamboo.specs.api.builders.BambooKey;
import com.atlassian.bamboo.specs.api.builders.BambooOid;
import com.atlassian.bamboo.specs.api.builders.plan.Job;
import com.atlassian.bamboo.specs.api.builders.plan.Plan;
import com.atlassian.bamboo.specs.api.builders.plan.Stage;
import com.atlassian.bamboo.specs.api.builders.plan.artifact.Artifact;
import com.atlassian.bamboo.specs.api.builders.plan.branches.BranchCleanup;
import com.atlassian.bamboo.specs.api.builders.plan.branches.PlanBranchManagement;
import com.atlassian.bamboo.specs.api.builders.project.Project;
import com.atlassian.bamboo.specs.api.builders.requirement.Requirement;
import com.atlassian.bamboo.specs.api.builders.task.AnyTask;
import com.atlassian.bamboo.specs.api.builders.trigger.AnyTrigger;
import com.atlassian.bamboo.specs.api.model.BambooKeyProperties;
import com.atlassian.bamboo.specs.api.model.BambooOidProperties;
import com.atlassian.bamboo.specs.api.model.plan.JobProperties;
import com.atlassian.bamboo.specs.api.model.plan.PlanProperties;
import com.atlassian.bamboo.specs.api.model.plan.StageProperties;
import com.atlassian.bamboo.specs.api.model.plan.artifact.ArtifactProperties;
import com.atlassian.bamboo.specs.api.model.plan.branches.PlanBranchManagementProperties;
import com.atlassian.bamboo.specs.api.model.plan.requirement.RequirementProperties;
import com.atlassian.bamboo.specs.api.model.project.ProjectProperties;
import com.atlassian.bamboo.specs.api.model.task.AnyTaskProperties;
import com.atlassian.bamboo.specs.api.model.trigger.AnyTriggerProperties;
import com.atlassian.bamboo.specs.builders.task.CheckoutItem;
import com.atlassian.bamboo.specs.builders.task.ScriptTask;
import com.atlassian.bamboo.specs.builders.task.TestParserTask;
import com.atlassian.bamboo.specs.builders.task.VcsCheckoutTask;
import com.atlassian.bamboo.specs.builders.trigger.BitbucketServerTrigger;
import com.atlassian.bamboo.specs.builders.trigger.RemoteTrigger;
import com.atlassian.bamboo.specs.model.task.ScriptTaskProperties;
import com.atlassian.bamboo.specs.model.task.TestParserTaskProperties;
import com.atlassian.bamboo.specs.model.task.VcsCheckoutTaskProperties;
import com.atlassian.bamboo.specs.model.trigger.BitbucketServerTriggerProperties;
import com.atlassian.bamboo.specs.model.trigger.RemoteTriggerProperties;
import com.atlassian.bamboo.specs.util.BambooServer;
import com.atlassian.bamboo.specs.util.MapBuilder;

@BambooSpec
public class PlanSpec {
    public static void main(String... argv) {
        //By default credentials are read from the '.credentials' file.
        BambooServer bambooServer = new BambooServer("http://localhost:8085");
        
        Plan rootObject = new Plan(new Project()
                    .oid(new BambooOid("1jx1kzc2o5xc1"))
                    .key(new BambooKey("PLAYG")),
                "DEP-801barcoding - move inline build script to bitbucket",
                new BambooKey("DMIBSTB"))
                .oid(new BambooOid("1jwrvrqpgcf7y"))
                .enabled(false)
                .stages(new Stage("Build and test")
                        .jobs(new Job("Build and test",
                                new BambooKey("JOB1"))
                                .artifacts(new Artifact()
                                        .name("zulu")
                                        .copyPattern("zulu")
                                        .location("build"),
                                    new Artifact()
                                        .name("lima")
                                        .copyPattern("lima")
                                        .location("build"))
                                .tasks(new VcsCheckoutTask()
                                        .description("Checkout")
                                        .checkoutItems(new CheckoutItem().defaultRepository())
                                        .cleanCheckout(true),
                                    new ScriptTask()
                                        .description("Build Script")
                                        .location(ScriptTaskProperties.Location.FILE)
                                        .fileFromPath("bamboo_scripts/build_scripts/barcoding_build.sh"),
                                    new TestParserTask(TestParserTaskProperties.TestType.JUNIT)
                                        .description("Tests")
                                        .resultDirectories("**/cram.xml"),
                                    new AnyTask(new AtlassianModule("ch.mibex.bamboo.sonar4bamboo:sonar4bamboo.sonarscannertask"))
                                        .description("runs static analysis for barcoding without any code coverage")
                                        .configuration(new MapBuilder()
                                                .put("incrementalFileForInclusionList", "")
                                                .put("chosenSonarConfigId", "4")
                                                .put("useGradleWrapper", "")
                                                .put("useNewGradleSonarQubePlugin", "")
                                                .put("sonarJavaSource", "")
                                                .put("sonarProjectName", "SAT-barcoding")
                                                .put("buildJdk", "JDK 1.8.0_101")
                                                .put("gradleWrapperLocation", "")
                                                .put("sonarLanguage", "")
                                                .put("sonarSources", "src,include")
                                                .put("useGlobalSonarServerConfig", "true")
                                                .put("incrementalMode", "")
                                                .put("failBuildForBrokenQualityGates", "")
                                                .put("sonarTests", "")
                                                .put("failBuildForSonarErrors", "")
                                                .put("sonarProjectVersion", "0.1.2.${bamboo.buildNumber}")
                                                .put("sonarBranch", "")
                                                .put("executable", "SonarScannerHome")
                                                .put("illegalBranchCharsReplacement", "_")
                                                .put("failBuildForTaskErrors", "true")
                                                .put("incrementalModeNotPossible", "incrementalModeRunFullAnalysis")
                                                .put("sonarJavaTarget", "")
                                                .put("environmentVariables", "")
                                                .put("incrementalModeGitBranchPattern", "")
                                                .put("legacyBranching", "")
                                                .put("replaceSpecialBranchChars", "")
                                                .put("additionalProperties", "-Dsonar.cfamily.build-wrapper-output.bypass=true")
                                                .put("autoBranch", "true")
                                                .put("sonarProjectKey", "SAT-barcoding")
                                                .put("incrementalModeBambooUser", "")
                                                .put("overrideSonarBuildConfig", "true")
                                                .put("workingSubDirectory", "")
                                                .build()))
                                .requirements(new Requirement("system.dist")
                                        .matchValue("redhat")
                                        .matchType(Requirement.MatchType.EQUALS))))
                .linkedRepositories("barcoding")
                
                .triggers(new BitbucketServerTrigger()
                        .name("Bitbucket Server repository triggered"),
                    new RemoteTrigger()
                        .name("Remote trigger"),
                    new AnyTrigger(new AtlassianModule("com.atlassian.bamboo.triggers.atlassian-bamboo-triggers:daily"))
                        .name("Single daily build")
                        .enabled(false)
                        .configuration(new MapBuilder()
                                .put("trigger.created.by.user", "mhsieh")
                                .put("repository.change.daily.buildTime", "00:00")
                                .build()))
                .planBranchManagement(new PlanBranchManagement()
                        .createForPullRequest()
                        .delete(new BranchCleanup()
                            .whenRemovedFromRepositoryAfterDays(7)
                            .whenInactiveInRepositoryAfterDays(30))
                        .notificationLikeParentPlan());
        
        bambooServer.publish(rootObject);
    }
}
